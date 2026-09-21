"""Build in Docker; load the module only in a disposable, networkless Linux VM."""
import argparse
from pathlib import Path
import platform
import shutil
import subprocess
import tempfile

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--image", default="message-slot-tests")
parser.add_argument("--skip-build", action="store_true")
args = parser.parse_args()
root = Path(__file__).resolve().parents[1]
qemu = shutil.which("qemu-system-aarch64")
if not qemu:
    parser.error("Install QEMU (qemu-system-aarch64) before running the VM test")
if not args.skip_build:
    subprocess.run(["docker", "build", "--platform", "linux/arm64", "-f", "docker/Dockerfile", "-t", args.image, "."], cwd=root, check=True)
with tempfile.TemporaryDirectory(prefix="message-slot-vm-") as directory:
    subprocess.run(["docker", "run", "--rm", "--platform", "linux/arm64", "--network", "none", "--cap-drop", "ALL", "--security-opt", "no-new-privileges", "-v", str(root)+":/project:ro", "-v", directory+":/output", args.image, "sh", "/project/scripts/build_vm.sh"], check=True, timeout=120)
    native_mac = platform.system() == "Darwin" and platform.machine() == "arm64"
    command = [qemu, "-machine", "virt", "-accel", "hvf" if native_mac else "tcg", "-cpu", "host" if native_mac else "cortex-a57", "-m", "512", "-smp", "1", "-nographic", "-monitor", "none", "-net", "none", "-no-reboot", "-kernel", directory+"/kernel", "-initrd", directory+"/initramfs.gz", "-append", "console=ttyAMA0 rdinit=/init panic=-1"]
    result = subprocess.run(command, capture_output=True, text=True, timeout=180)
    print(result.stdout)
    if result.returncode or "VM_TEST_PASSED" not in result.stdout or "VM_TEST_FAILED" in result.stdout:
        raise SystemExit("Guest validation failed: " + result.stderr[-2000:])
print("PASS: module loaded, device/CLI checks passed, module unloaded and reloaded in an isolated guest")
