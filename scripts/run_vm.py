"""Build and run Message Slot in a disposable, networkless Linux guest."""

import argparse
import platform
import shutil
import subprocess
import tempfile
from pathlib import Path

parser = argparse.ArgumentParser(description=__doc__)
mode = parser.add_mutually_exclusive_group()
mode.add_argument(
    "--test", action="store_true", help="Run the device tests and shut down"
)
mode.add_argument(
    "--build-only",
    action="store_true",
    help="Save kernel/initramfs in build/vm without booting",
)
parser.add_argument("--image", default="message-slot-tests")
parser.add_argument(
    "--skip-build",
    action="store_true",
    help="Reuse the existing Docker toolchain image",
)
args = parser.parse_args()
root = Path(__file__).resolve().parents[1]
qemu = shutil.which("qemu-system-aarch64")
if not args.build_only and not qemu:
    parser.error("Install QEMU (qemu-system-aarch64) before running the VM")
if not shutil.which("docker"):
    parser.error("Install and start Docker before building the guest")

if not args.skip_build:
    subprocess.run(
        [
            "docker",
            "build",
            "--platform",
            "linux/arm64",
            "-f",
            "docker/Dockerfile",
            "-t",
            args.image,
            ".",
        ],
        cwd=root,
        check=True,
        timeout=900,
    )


def build_guest(directory):
    init = "tests/vm-init.sh" if args.test else "scripts/vm-init.sh"
    subprocess.run(
        [
            "docker",
            "run",
            "--rm",
            "--platform",
            "linux/arm64",
            "--network",
            "none",
            "--cap-drop",
            "ALL",
            "--security-opt",
            "no-new-privileges",
            "-e",
            "VM_INIT=" + init,
            "-v",
            str(root) + ":/project:ro",
            "-v",
            str(directory) + ":/output",
            args.image,
            "sh",
            "/project/scripts/build_vm.sh",
        ],
        check=True,
        timeout=120,
    )


def boot(directory):
    native_mac = platform.system() == "Darwin" and platform.machine() == "arm64"
    command = [
        qemu,
        "-machine",
        "virt",
        "-accel",
        "hvf" if native_mac else "tcg",
        "-cpu",
        "host" if native_mac else "cortex-a57",
        "-m",
        "512",
        "-smp",
        "1",
        "-nographic",
        "-monitor",
        "none",
        "-net",
        "none",
        "-no-reboot",
        "-kernel",
        str(directory / "kernel"),
        "-initrd",
        str(directory / "initramfs.gz"),
        "-append",
        "console=ttyAMA0 rdinit=/init panic=-1 quiet loglevel=3",
    ]
    if args.test:
        result = subprocess.run(command, capture_output=True, text=True, timeout=180)
        print(result.stdout)
        if (
            result.returncode
            or "VM_TEST_PASSED" not in result.stdout
            or "VM_TEST_FAILED" in result.stdout
        ):
            raise SystemExit("Guest tests failed: " + result.stderr[-2000:])
        print("PASS: device/CLI checks, unload and reload in an isolated Linux guest")
    else:
        subprocess.run(command, check=True)


if args.build_only:
    directory = root / "build" / "vm"
    directory.mkdir(parents=True, exist_ok=True)
    build_guest(directory)
    print(
        "Built build/vm/kernel and build/vm/initramfs.gz. Use make run to open a fresh guest."
    )
else:
    with tempfile.TemporaryDirectory(prefix="message-slot-vm-") as temporary:
        directory = Path(temporary)
        build_guest(directory)
        boot(directory)
