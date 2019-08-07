import subprocess
import shlex
import signal
import os
import re
import tempfile
import time
from contextlib import contextmanager


@contextmanager
def tempFifo(fname):
    """Context Manager for creating named pipes with temporary names."""
    tmpdir = tempfile.mkdtemp()
    fpath = os.path.join(tmpdir, fname)  # Temporary filename
    os.mkfifo(fpath)  # Create FIFO
    yield fpath
    os.unlink(fpath)  # Remove file
    os.rmdir(tmpdir)  # Remove directory


def run(fifo_path):
    tpl = '../../nsjail --config nsjail-23.cfg --cwd "{cwd}" --log "{log}" -- ./23'
    cmd = tpl.format(
        cwd=os.getcwd(),
        log=fifo_path)

    pTarget = subprocess.Popen(shlex.split(cmd))
    pDebug = subprocess.Popen('./23m', stdin=subprocess.PIPE)

    pid = None
    log = open(fifo_path, 'r')
    for ln in log:
        print('>>> LOG: ' + ln.strip())
        grp = re.search(r'(\d+):process_spawned', ln)
        if grp is None:
            continue
        pid = int(grp.group(1))
        break

    print('!!! PID=', pid)
    pDebug.stdin.write('{}\n'.format(pid).encode())
    pDebug.stdin.flush()
    pDebug.wait()
    pTarget.wait()

    print('!!! PROCESS END')

    for ln in log:
        print('>>> LOG: ' + ln.strip())

    log.close()

if __name__ == '__main__':
    with tempFifo('runner') as fifo_path:
        run(fifo_path)
