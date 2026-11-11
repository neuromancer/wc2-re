#!/usr/bin/env python3
"""Run seeded WC2/SO1/SO2 flight checks in disposable DOS or Saga installations.

Adapted from wc1-re/bin/runGameTests.py. Each child exercises one mission's
active nav points, with a replay command and a separate log for every case.
"""
import argparse
import concurrent.futures
import hashlib
import json
import os
from pathlib import Path
import queue
import secrets
import shlex
import shutil
import subprocess
import sys
import tempfile


CAMPAIGNS = ("wc2", "so1", "so2")


def positive(value):
    number = int(value)
    if number < 1:
        raise argparse.ArgumentTypeError("must be positive")
    return number


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("probe", type=Path)
    parser.add_argument("data", type=Path, help="installation containing GAMEDAT (STREAMS optional)")
    parser.add_argument("--campaign", choices=(*CAMPAIGNS, "all"), default="all")
    parser.add_argument("--seed", type=int, help="32-bit seed; chosen and printed if omitted")
    parser.add_argument("--runs", type=positive, default=1, help="consecutive seeds")
    parser.add_argument("--ticks", type=positive, default=600, help="flight ticks per nav point, 1..3600")
    parser.add_argument("--case", help="one mission, e.g. so2:3:1 (mission is zero-based)")
    parser.add_argument("--nav", type=int, help="one nav index, 0..9; requires --case")
    parser.add_argument("--timeout", type=positive, default=120, help="seconds per child")
    parser.add_argument("--jobs", type=positive, default=1, help="parallel isolated workers")
    parser.add_argument("--keep-going", action="store_true", help="finish the sweep after a failure")
    parser.add_argument("--log-dir", type=Path, help="new directory for logs and results.json")
    args = parser.parse_args()
    if args.ticks > 3600 or args.seed is not None and not 0 <= args.seed <= 0xffffffff:
        parser.error("ticks must be in 1..3600 and seed in 0..4294967295")
    if args.nav is not None and (not args.case or not 0 <= args.nav < 10):
        parser.error("--nav must be in 0..9 and requires --case")
    selected_campaign = None
    if args.case:
        try:
            selected_campaign, series, mission = args.case.split(":")
            if selected_campaign not in CAMPAIGNS or int(series) < 1 or not 0 <= int(mission) < 4:
                raise ValueError()
        except ValueError:
            parser.error("--case must have the form wc2:1:0, so1:1:0 or so2:1:0")
        if args.campaign not in ("all", selected_campaign):
            parser.error("--case and --campaign disagree")
    probe = args.probe.resolve()
    if not probe.is_file():
        parser.error(f"build the probe first: {probe}")
    installation = args.data.resolve()
    directories = {path.name.upper(): path for path in installation.iterdir() if path.is_dir()}
    if "GAMEDAT" not in directories:
        parser.error("supply an installation containing GAMEDAT")
    seed = args.seed if args.seed is not None else secrets.randbits(32)
    if args.log_dir:
        log_dir = args.log_dir.resolve()
        log_dir.mkdir(parents=True, exist_ok=False)
    else:
        log_dir = Path(tempfile.mkdtemp(prefix=f"flight-{seed}-", dir=probe.parent))
    env = dict(os.environ, SDL_VIDEODRIVER="dummy", SDL_AUDIODRIVER="dummy",
               ASAN_OPTIONS="detect_leaks=0:halt_on_error=1:abort_on_error=1",
               UBSAN_OPTIONS="halt_on_error=1:print_stacktrace=1")
    excluded = {"WC2.CFG", "WINGCMDR.CFG", "TAPE.TMP", "MEM.$$$", "ERR.$$$",
                "LOGFILE.TXT", "PROBE-COMPLETE"}

    def ignore(_directory, names):
        return [name for name in names if name.upper() in excluded or
                name.upper().startswith(("SAVEGAME", "TEMPGLOB"))]

    def run(root, arguments, log):
        complete = root / "GAMEDAT" / "probe-complete"
        complete.unlink(missing_ok=True)
        with log.open("w") as output:
            try:
                result = subprocess.run([str(probe), str(root), *map(str, arguments)],
                                        cwd=root, env=env, timeout=args.timeout,
                                        stdout=output, stderr=subprocess.STDOUT)
                error = None if result.returncode == 0 else f"exit {result.returncode}"
            except subprocess.TimeoutExpired:
                error = f"timeout after {args.timeout}s"
        text = log.read_text(errors="replace")
        if not error and ("ERROR: AddressSanitizer" in text or "runtime error:" in text):
            error = "sanitizer report"
        # Some original fatal game paths exit with status zero.
        if not error and not complete.is_file():
            error = "exited without completion marker"
        return error, text

    results = []
    metadata = {"probe": str(probe), "probe_sha256": hashlib.sha256(probe.read_bytes()).hexdigest(),
                "data": str(installation), "seed": seed, "ticks": args.ticks, "results": results}
    print(f"Seed {seed}; logs: {log_dir}", flush=True)
    with tempfile.TemporaryDirectory(prefix="wc2-flight-") as scratch:
        root = Path(scratch)
        template = root / "template"
        shutil.copytree(directories["GAMEDAT"], template / "GAMEDAT", ignore=ignore)
        if "STREAMS" in directories:
            shutil.copytree(directories["STREAMS"], template / "STREAMS")
        files = {path.name.upper() for path in (template / "GAMEDAT").iterdir()}
        cases = []
        for campaign, name in enumerate(CAMPAIGNS):
            if selected_campaign and name != selected_campaign or args.campaign not in ("all", name):
                continue
            if campaign and f"CAMPAIGN.S0{campaign}" not in files:
                if args.campaign == name or selected_campaign == name:
                    raise RuntimeError(f"{name} data not installed")
                continue
            log = log_dir / f"list-{name}.log"
            error, text = run(template, [campaign, "--list"], log)
            if error:
                raise RuntimeError(f"Cannot enumerate {name}: {error}; see {log}\n{text[-3000:]}")
            for line in text.splitlines():
                if line.startswith("CASE "):
                    c, s, m = map(int, line.split()[1:])
                    label = f"{CAMPAIGNS[c]}:{s}:{m}"
                    if args.case in (None, label):
                        cases.append((label, c, s, m))
        if not cases:
            raise RuntimeError("no matching missions")
        pending = queue.Queue()
        for index in range(min(args.jobs, len(cases) * args.runs)):
            worker = root / f"worker-{index}"
            shutil.copytree(template, worker, ignore=ignore)
            pending.put(worker)

        def check(case):
            label, campaign, series, mission, run_seed = case
            worker = pending.get()
            log = log_dir / f"{label.replace(':', '-')}-seed-{run_seed}.log"
            replay = [sys.executable, str(Path(__file__).resolve()), str(probe), str(installation),
                      "--case", label, "--seed", str(run_seed), "--ticks", str(args.ticks),
                      "--timeout", str(args.timeout)]
            arguments = [campaign, series, mission, run_seed, args.ticks]
            if args.nav is not None:
                replay += ["--nav", str(args.nav)]
                arguments.append(args.nav)
            try:
                error, text = run(worker, arguments, log)
            finally:
                pending.put(worker)
            result = {"case": label, "seed": run_seed, "error": error,
                      "replay": shlex.join(replay), "log": str(log),
                      "encounters": text.count("END ticks=")}
            print(f"{'FAIL' if error else 'PASS'} seed={run_seed} {label}" +
                  (f": {error}; {log}" if error else f" ({result['encounters']} navs)"), flush=True)
            if error:
                print(f"Replay: {result['replay']}\n" + "\n".join(text.splitlines()[-35:]), flush=True)
            return result

        work = [(*case, (seed + run_index) & 0xffffffff)
                for run_index in range(args.runs) for case in cases]
        print(f"{len(cases)} missions x {args.runs} seeds; {pending.qsize()} workers", flush=True)
        with concurrent.futures.ThreadPoolExecutor(max_workers=pending.qsize()) as pool:
            if args.keep_going:
                futures = [pool.submit(check, case) for case in work]
                for future in concurrent.futures.as_completed(futures):
                    results.append(future.result())
                    (log_dir / "results.json").write_text(json.dumps(metadata, indent=2) + "\n")
            else:
                # Fail-fast only schedules one worker-sized batch at a time.
                for start in range(0, len(work), args.jobs):
                    results.extend(pool.map(check, work[start:start + args.jobs]))
                    (log_dir / "results.json").write_text(json.dumps(metadata, indent=2) + "\n")
                    if any(result["error"] for result in results):
                        break
    failed = sum(result["error"] is not None for result in results)
    encounters = sum(result["encounters"] for result in results)
    print(f"{len(results) - failed}/{len(results)} mission runs passed; "
          f"{encounters} nav encounters completed; results: {log_dir / 'results.json'}")
    return 1 if failed else 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, RuntimeError, subprocess.SubprocessError) as error:
        raise SystemExit(str(error)) from error
