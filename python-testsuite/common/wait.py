from time import time, sleep

def waitFor(expr, timeout):
    endtime = time() + timeout
    while time() < endtime:
        if expr():
            return True
        sleep(0.05)
    return False

