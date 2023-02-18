from pathlib import Path
from subprocess import PIPE, STDOUT, run

HERE = Path(__file__).resolve().parent
TEST = HERE.parent
ROOT = TEST.parent
SRC = ROOT / "src"


class CompilationError(Exception):
    pass


def compile(source: Path, cflags=[], ldadd=[]):
    binary = source.with_suffix(".so")

    result = run(
        ["gcc", "-shared", *cflags, "-o", str(binary), str(source), *ldadd],
        stdout=PIPE,
        stderr=STDOUT,
        cwd=SRC,
    )

    if result.returncode == 0:
        return

    raise CompilationError(result.stdout.decode())
