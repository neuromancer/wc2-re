#!/usr/bin/env python3
"""Fuzz WC2, SO1 or SO2 flights with libFuzzer in a disposable installation.

Adapted from wc1-re/bin/runGameTests.py. Each fuzzer process flies one
campaign, because each campaign loads its own ship and mission data; run the
campaigns in separate processes. libFuzzer flags follow --.
"""
import argparse
import hashlib
import os
from pathlib import Path
import shlex
import shutil
import subprocess
import sys
import tempfile


CAMPAIGNS = ("wc2", "so1", "so2")
SERIES = (12, 5, 5)


def positive(value):
    number = int(value)
    if number < 1:
        raise argparse.ArgumentTypeError("must be positive")
    return number


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("probe", type=Path, help="flight-fuzzer built by make fuzz-flight")
    parser.add_argument("data", type=Path, help="installation containing GAMEDAT (STREAMS optional)")
    parser.add_argument("--campaign", choices=CAMPAIGNS, help="defaults to --case's campaign, else wc2")
    parser.add_argument("--ticks", type=positive, default=600,
                        help="maximum controlled flight ticks per input (1..3600)")
    parser.add_argument("--case", help="restrict to one mission, e.g. so2:3:1 (mission is zero-based)")
    parser.add_argument("--timeout", type=positive, default=120, help="seconds per input")
    parser.add_argument("--corpus", type=Path, help="persistent input corpus; defaults beside the fuzzer")
    parser.add_argument("--replay", type=Path, help="replay a saved libFuzzer input without mutations")
    arguments = sys.argv[1:]
    split = arguments.index("--") if "--" in arguments else len(arguments)
    args = parser.parse_args(arguments[:split])
    fuzz_flags = arguments[split + 1:]
    if any(not flag.startswith("-") for flag in fuzz_flags):
        parser.error("use --corpus or --replay for input paths; only libFuzzer flags follow --")
    if args.ticks > 3600:
        parser.error("ticks must be in 1..3600")
    case = None
    if args.case:
        try:
            name, series, mission = args.case.split(":")
            case = (CAMPAIGNS.index(name), int(series), int(mission))
            if not 1 <= case[1] <= SERIES[case[0]] or not 0 <= case[2] < 4:
                raise ValueError
            if args.campaign not in (None, name):
                raise ValueError
        except ValueError:
            parser.error("--case must be campaign:series:mission, e.g. wc2:1:0, matching --campaign")
    campaign = case[0] if case else CAMPAIGNS.index(args.campaign or "wc2")
    name = CAMPAIGNS[campaign]
    probe = args.probe.resolve()
    if not probe.is_file():
        parser.error(f"build the fuzzer first (make fuzz-flight): {probe}")
    if args.replay and not args.replay.is_file():
        parser.error(f"replay input does not exist: {args.replay}")
    installation = args.data.resolve()
    directories = ({path.name.upper(): path for path in installation.iterdir() if path.is_dir()}
                   if installation.is_dir() else {})
    if "GAMEDAT" not in directories:
        parser.error("supply an installation containing GAMEDAT")
    files = {path.name.upper() for path in directories["GAMEDAT"].iterdir()}
    if campaign and f"CAMPAIGN.S0{campaign}" not in files:
        parser.error(f"{name} is not installed in {installation}")
    env = dict(os.environ, SDL_VIDEODRIVER="dummy", SDL_AUDIODRIVER="dummy",
               ASAN_OPTIONS="detect_leaks=0", UBSAN_OPTIONS="halt_on_error=0:print_stacktrace=1")
    for variable in ("WC2_FLIGHT_CASE", "WC2_FLIGHT_LIST", "WC2_FLIGHT_SEEDS"):
        env.pop(variable, None)
    env["WC2_FLIGHT_CAMPAIGN"] = str(campaign)
    env["WC2_FLIGHT_TICKS"] = str(args.ticks)
    if case:
        env["WC2_FLIGHT_CASE"] = ":".join(map(str, case))
    # Saves, settings and the game's logs stay out of the disposable copy.
    excluded = {"WC2.CFG", "WINGCMDR.CFG", "TAPE.TMP", "MEM.$$$", "ERR.$$$",
                "LOGFILE.TXT", "PROBE-COMPLETE"}

    def ignore(_directory, names):
        return [entry for entry in names if entry.upper() in excluded or
                entry.upper().startswith(("SAVEGAME", "TEMPGLOB"))]

    with tempfile.TemporaryDirectory(prefix=f"wc2-fuzz-{name}-") as scratch:
        root = Path(scratch)
        shutil.copytree(directories["GAMEDAT"], root / "GAMEDAT", ignore=ignore)
        if "STREAMS" in directories:
            shutil.copytree(directories["STREAMS"], root / "STREAMS")
        complete = root / "GAMEDAT" / "probe-complete"
        env["WC2_TEST_ROOT"] = scratch
        inventory = subprocess.run([str(probe)], env=dict(env, WC2_FLIGHT_LIST="1"),
                                   timeout=600, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                   text=True, errors="replace", check=True)
        if not complete.is_file():
            raise RuntimeError("flight inventory exited before completing its checks:\n" +
                               inventory.stdout[-3000:])
        lines = inventory.stdout.splitlines()
        cases = [tuple(map(int, line.split()[1:])) for line in lines if line.startswith("CASE ")]
        print(f"Flight missions: {name} {len(cases)}", flush=True)
        if not cases:
            parser.error("no matching missions; check --case")
        corpus = (args.corpus or probe.parent / f"flight-corpus-{name}").resolve()
        artifacts = probe.parent / f"flight-artifacts-{name}"
        artifacts.mkdir(parents=True, exist_ok=True)
        forked = any(flag.startswith(("-fork=", "-jobs=")) for flag in fuzz_flags)
        if forked:
            # Fork mode discards a crashed job's output, and a crash can need
            # the inputs that job ran before; keep every sanitizer report.
            reports = probe.parent / f"flight-reports-{name}"
            reports.mkdir(parents=True, exist_ok=True)
            env["ASAN_OPTIONS"] += f":log_path={reports / 'report'}"
            print(f"Sanitizer reports: {reports}", flush=True)
        if args.replay:
            header = tuple(args.replay.read_bytes()[:3])
            if header not in cases:
                parser.error(f"replay input is for {header}, not one of the selected {name} missions")
            inputs = [str(args.replay.resolve())]
        else:
            corpus.mkdir(parents=True, exist_ok=True)
            # The fuzzer adds small seeds for every selected mission, start leg,
            # cockpit start and active nav point.
            env["WC2_FLIGHT_SEEDS"] = str(corpus)
            inputs = [str(corpus)]
        command = [str(probe), *inputs, f"-max_len={8 + 2 * args.ticks}",
                   f"-timeout={args.timeout}", "-detect_leaks=0",
                   f"-artifact_prefix={artifacts}{os.sep}"]
        # The game's console output slows every input; libFuzzer keeps its own
        # status and the sanitizer reports. Replays keep everything. Closing
        # stderr would also send the reports fork mode keeps to a job log.
        if not args.replay and not any(flag.startswith("-close_fd_mask=") for flag in fuzz_flags):
            command.append("-close_fd_mask=1" if forked else "-close_fd_mask=3")
        command += fuzz_flags
        replay = [sys.executable, str(Path(__file__).resolve()), str(probe), str(installation),
                  "--campaign", name, "--ticks", str(args.ticks),
                  "--replay", str(artifacts / "crash-<hash>")]
        with tempfile.NamedTemporaryFile(mode="w", encoding="utf-8", prefix=f"flight-{name}-",
                                         suffix=".log", dir=probe.parent, delete=False) as log:
            print(f"Full flight log: {log.name}", flush=True)
            for line in (f"Build SHA256: {hashlib.sha256(probe.read_bytes()).hexdigest()}",
                         f"Data: {installation}", f"UBSAN_OPTIONS={env['UBSAN_OPTIONS']}",
                         f"Replay with the same build/data: {shlex.join(replay)}",
                         shlex.join(command)):
                print(line, flush=True)
                log.write(line + "\n")
            log.write(inventory.stdout)
            with subprocess.Popen(command, env=env, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                  text=True, encoding="utf-8", errors="replace") as process:
                for line in process.stdout:
                    print(line, end="", flush=True)
                    log.write(line)
                    log.flush()
                status = process.wait()
            log.write(f"\nExit status: {status}\n")
            if status:
                raise RuntimeError(f"Flight fuzzer exited with status {status}; full report: {log.name}")


if __name__ == "__main__":
    try:
        main()
    except (OSError, RuntimeError, subprocess.SubprocessError) as error:
        if isinstance(error, subprocess.CalledProcessError) and error.stdout:
            print(error.stdout[-6000:], file=sys.stderr, end="")
        raise SystemExit(str(error)) from error
