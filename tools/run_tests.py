#!/usr/bin/env python3
"""Run the AS&S Game test ROM in mGBA and report results.

Usage: python3 tools/run_tests.py [test_rom.gba]

The test ROM exercises all game systems and writes results to SRAM,
which mGBA persists as a .sav file.  This script:
  1. Removes any stale .sav file
  2. Launches mGBA with the test ROM
  3. Waits for tests to complete (the ROM writes a 0xAA marker when done)
  4. Reads the .sav file and reports pass/fail

SRAM result protocol (written by test_main.c):
  Byte 0-3: magic "TEST"
  Byte 4:   total test count
  Byte 5:   pass count
  Byte 6:   fail count
  Byte 7:   done flag (0xAA = complete)
  Byte 8+:  per-test results (0=pass, 1=fail)
"""
import os
import signal
import subprocess
import sys
import time

# Defaults
DEFAULT_ROM = "ass_game_test.gba"
SAV_SUFFIX = ".sav"
MGBA_BIN = os.environ.get("MGBA", "mgba")
TIMEOUT_SECS = 10       # Max time to wait for test ROM to finish
POLL_INTERVAL = 0.5     # How often to check for .sav

DONE_MARKER = 0xAA
MAGIC = b"TEST"


def find_sav(rom_path):
    """Return the .sav path mGBA would create for the given ROM."""
    base, _ = os.path.splitext(rom_path)
    return base + SAV_SUFFIX


def read_sav(sav_path):
    """Parse the SRAM result file.  Returns (total, passed, failed, per_test)."""
    with open(sav_path, "rb") as f:
        data = f.read()

    if len(data) < 8:
        raise ValueError(f".sav file too small ({len(data)} bytes)")

    magic = data[0:4]
    if magic != MAGIC:
        raise ValueError(f"Bad magic: {magic!r} (expected {MAGIC!r})")

    total = data[4]
    passed = data[5]
    failed = data[6]
    done = data[7]

    if done != DONE_MARKER:
        raise ValueError(f"Tests did not finish (done marker=0x{done:02X}, expected 0xAA)")

    per_test = []
    for i in range(total):
        if 8 + i < len(data):
            per_test.append("PASS" if data[8 + i] == 0 else "FAIL")
        else:
            per_test.append("???")

    return total, passed, failed, per_test


def main():
    rom = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_ROM

    if not os.path.exists(rom):
        print(f"ERROR: ROM not found: {rom}", file=sys.stderr)
        print(f"Run 'make test-rom' first to build the test ROM.", file=sys.stderr)
        sys.exit(2)

    sav = find_sav(rom)

    # Remove stale .sav so we get a fresh result
    if os.path.exists(sav):
        os.remove(sav)
        print(f"Removed stale {sav}")

    # Launch mGBA
    print(f"Launching mGBA with {rom}...")
    proc = subprocess.Popen(
        [MGBA_BIN, "-l", "4", rom],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )

    # Wait for the .sav to appear and be valid
    start = time.time()
    result = None
    while time.time() - start < TIMEOUT_SECS:
        time.sleep(POLL_INTERVAL)
        if os.path.exists(sav):
            try:
                result = read_sav(sav)
                break  # Got a valid result
            except ValueError:
                pass  # .sav exists but tests not done yet

    # Shut down mGBA
    try:
        proc.send_signal(signal.SIGTERM)
        proc.wait(timeout=5)
    except Exception:
        proc.kill()
        proc.wait()

    if result is None:
        # Try one final read — mGBA flushes SRAM on exit
        if os.path.exists(sav):
            try:
                result = read_sav(sav)
            except ValueError as e:
                print(f"\nERROR: {e}", file=sys.stderr)
                sys.exit(1)
        else:
            print(f"\nERROR: No .sav file produced after {TIMEOUT_SECS}s.", file=sys.stderr)
            print("Make sure mGBA is installed and the test ROM is valid.", file=sys.stderr)
            sys.exit(1)

    total, passed, failed, per_test = result

    # Report
    print()
    print("=" * 50)
    print(f"  AS&S Game Test Results")
    print("=" * 50)
    print(f"  Total:  {total}")
    print(f"  Passed: {passed}")
    print(f"  Failed: {failed}")
    print("=" * 50)

    if failed > 0:
        print("\nFailed tests:")
        for i, r in enumerate(per_test):
            if r == "FAIL":
                print(f"  Test #{i}: FAIL")

    print()
    if failed == 0:
        print("ALL TESTS PASSED")
        sys.exit(0)
    else:
        print(f"{failed} TEST(S) FAILED")
        sys.exit(1)


if __name__ == "__main__":
    main()
