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
        # be warned that the pipe might be full, and thus blocks
        # the main process, which causes deadlock, if the log is
        # too long (which is unlikely here)
        break

    print('!!! PID=', pid)
    # sleep deliberately to eliminate the race condition:
    # if the child does not stop, then the tracer must fail to attach
    time.sleep(0.5)
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
