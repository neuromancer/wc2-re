#!/usr/bin/env python3
"""Record the SDL2 port's window with OBS, start to finish, unattended.

Launches the port, points an OBS window-capture at it, sizes the canvas to the
window so nothing but the game is in frame, records, and puts everything back.

    bin/record-modern.py --seconds 30
    bin/record-modern.py --run-dir data/wc2-full --args --enhanced

Needs OBS with its WebSocket server enabled once, by hand, under
Tools -> WebSocket Server Settings.  The port and password are read from OBS's
own config, so nothing has to be repeated here.  macOS also needs Screen
Recording permission granted to OBS, which it will prompt for on first capture.
"""

import argparse
import asyncio
import base64
import hashlib
import json
import os
import pathlib
import shutil
import signal
import subprocess
import sys
import time

import websockets

OBS_CONFIG = pathlib.Path.home() / (
    "Library/Application Support/obs-studio/plugin_config/obs-websocket/"
    "config.json")
WINDOW_PREFIX = "[wc2-modern]"
SCENE = "wc2-capture"
SOURCE = "wc2-window"


def obs_settings():
    if not OBS_CONFIG.is_file():
        raise SystemExit(f"OBS websocket config not found: {OBS_CONFIG}")
    config = json.loads(OBS_CONFIG.read_text())
    if not config.get("server_enabled"):
        raise SystemExit(
            "OBS's WebSocket server is disabled.  Enable it under "
            "Tools -> WebSocket Server Settings, then run this again.")
    return config.get("server_port", 4455), config.get("server_password", "")


class Obs:
    def __init__(self, port, password):
        self.url = f"ws://127.0.0.1:{port}"
        self.password = password
        self.socket = None
        self.counter = 0

    async def __aenter__(self):
        self.socket = await websockets.connect(self.url, max_size=None)
        hello = json.loads(await self.socket.recv())
        identify = {"op": 1, "d": {"rpcVersion": 1}}
        auth = hello["d"].get("authentication")
        if auth:
            secret = base64.b64encode(hashlib.sha256(
                (self.password + auth["salt"]).encode()).digest())
            identify["d"]["authentication"] = base64.b64encode(
                hashlib.sha256(
                    secret + auth["challenge"].encode()).digest()).decode()
        await self.socket.send(json.dumps(identify))
        while json.loads(await self.socket.recv())["op"] != 2:
            pass
        return self

    async def __aexit__(self, *_):
        await self.socket.close()

    async def call(self, request, data=None, required=True):
        self.counter += 1
        token = str(self.counter)
        await self.socket.send(json.dumps({"op": 6, "d": {
            "requestType": request, "requestId": token,
            "requestData": data or {}}}))
        while True:
            message = json.loads(await self.socket.recv())
            if message["op"] == 7 and message["d"]["requestId"] == token:
                status = message["d"]["requestStatus"]
                if required and not status["result"]:
                    raise SystemExit(
                        f"{request} failed: {status.get('comment')}")
                return message["d"].get("responseData") or {}


async def find_window(obs):
    items = (await obs.call("GetInputPropertiesListPropertyItems",
                            {"inputName": SOURCE,
                             "propertyName": "window"}))["propertyItems"]
    for item in items:
        if (item.get("itemName") or "").startswith(WINDOW_PREFIX):
            return item["itemValue"]
    return None


async def record(options, game):
    port, password = obs_settings()
    async with Obs(port, password) as obs:
        scenes = [s["sceneName"]
                  for s in (await obs.call("GetSceneList"))["scenes"]]
        if SCENE not in scenes:
            await obs.call("CreateScene", {"sceneName": SCENE})
        inputs = [i["inputName"]
                  for i in (await obs.call("GetInputList"))["inputs"]]
        if SOURCE not in inputs:
            await obs.call("CreateInput", {
                "sceneName": SCENE, "inputName": SOURCE,
                "inputKind": "screen_capture",
                "inputSettings": {"type": 1, "show_cursor": False}})

        # The window id is new on every launch, so bind by title each time.
        window = None
        deadline = time.monotonic() + 30
        while window is None and time.monotonic() < deadline:
            window = await find_window(obs)
            if window is None:
                await asyncio.sleep(1)
        if window is None:
            raise SystemExit(
                f"No window titled {WINDOW_PREFIX} appeared within 30s.")
        await obs.call("SetInputSettings", {
            "inputName": SOURCE, "overlay": True,
            "inputSettings": {"type": 1, "window": window,
                              "show_cursor": False}})

        # Match the canvas to the window's backing size so the recording is
        # the window and nothing else, at its own pixel scale.
        transform = (await obs.call("GetSceneItemTransform", {
            "sceneName": SCENE, "sceneItemId": 1}))["sceneItemTransform"]
        width = int(transform["sourceWidth"])
        height = int(transform["sourceHeight"])
        previous = await obs.call("GetVideoSettings")
        if width and height:
            await obs.call("SetVideoSettings", {
                "baseWidth": width, "baseHeight": height,
                "outputWidth": width, "outputHeight": height,
                "fpsNumerator": previous["fpsNumerator"],
                "fpsDenominator": previous["fpsDenominator"]})
        await obs.call("SetSceneItemTransform", {
            "sceneName": SCENE, "sceneItemId": 1,
            "sceneItemTransform": {
                "positionX": 0.0, "positionY": 0.0,
                "scaleX": 1.0, "scaleY": 1.0,
                "cropTop": 0, "cropBottom": 0,
                "cropLeft": 0, "cropRight": 0}})
        await obs.call("SetCurrentProgramScene", {"sceneName": SCENE})

        directory = await obs.call("GetRecordDirectory")
        if options.output_dir:
            await obs.call("SetRecordDirectory",
                           {"recordDirectory": options.output_dir})

        await obs.call("StartRecord")
        print(f"recording {width}x{height} for {options.seconds}s", flush=True)
        try:
            game.wait(timeout=options.seconds)
            print("the game exited on its own", flush=True)
        except subprocess.TimeoutExpired:
            pass
        result = await obs.call("StopRecord")

        # StopRecord returns before the output has finished shutting down, and
        # the canvas cannot be resized until it has.
        deadline = time.monotonic() + 30
        while time.monotonic() < deadline:
            if not (await obs.call("GetRecordStatus"))["outputActive"]:
                break
            await asyncio.sleep(0.5)

        # Leave OBS as it was found.
        await obs.call("SetVideoSettings", {
            "baseWidth": previous["baseWidth"],
            "baseHeight": previous["baseHeight"],
            "outputWidth": previous["outputWidth"],
            "outputHeight": previous["outputHeight"],
            "fpsNumerator": previous["fpsNumerator"],
            "fpsDenominator": previous["fpsDenominator"]})
        if options.output_dir:
            await obs.call("SetRecordDirectory", directory)
        return result.get("outputPath")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--seconds", type=int, default=30)
    parser.add_argument("--run-dir", default="data/wc2-full",
                        help="directory the port runs from")
    parser.add_argument("--binary", default="out-modern/wc2-modern")
    parser.add_argument("--output-dir", default=None,
                        help="where to write the recording "
                             "(default: OBS's own setting)")
    parser.add_argument("--game-output", action="store_true",
                        help="let the port's own stdout through")
    parser.add_argument("--args", nargs=argparse.REMAINDER, default=[],
                        help="arguments passed on to the port")
    options = parser.parse_args()

    root = pathlib.Path(__file__).resolve().parent.parent
    binary = (root / options.binary).resolve()
    run_dir = (root / options.run_dir).resolve()
    if not binary.is_file():
        raise SystemExit(f"No port binary at {binary}; run `make modern`.")
    if not run_dir.is_dir():
        raise SystemExit(f"No run directory at {run_dir}.")
    if options.output_dir:
        options.output_dir = str(pathlib.Path(options.output_dir).resolve())

    if not shutil.which("pgrep") or subprocess.run(
            ["pgrep", "-x", "OBS"], capture_output=True).returncode != 0:
        print("starting OBS", flush=True)
        subprocess.run(["open", "-a", "OBS"], check=True)
        port, _ = obs_settings()
        deadline = time.monotonic() + 60
        while time.monotonic() < deadline:
            probe = subprocess.run(["nc", "-z", "127.0.0.1", str(port)],
                                   capture_output=True)
            if probe.returncode == 0:
                break
            time.sleep(2)

    environment = dict(os.environ, ASAN_OPTIONS="detect_leaks=0")
    output = None if options.game_output else subprocess.DEVNULL
    game = subprocess.Popen([str(binary)] + options.args,
                            cwd=str(run_dir), env=environment,
                            stdout=output, stderr=output)
    try:
        path = asyncio.run(record(options, game))
    finally:
        if game.poll() is None:
            game.send_signal(signal.SIGTERM)
            try:
                game.wait(timeout=5)
            except subprocess.TimeoutExpired:
                game.kill()
    print(path or "(OBS reported no output path)")


if __name__ == "__main__":
    main()
