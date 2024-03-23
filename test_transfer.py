import sys
import os
import numpy
import matplotlib.pyplot as plt 
from subprocess import Popen, PIPE

runsCount = -1

args_message = \
"""
Аргументы:
    1. Количество различных конфигураций ядер, которые нужно протестировать.
    2. Конфигурации ядер.
    3. Число запусков програмы.
    4. Аргументы запуска тестируемой программы.
Пример:
    ./test_transfer.py 4 1 2 3 4 ./ppi 1e9
    Запустить программу ./ppi с аргументов 1e9 на количестве ядер 1, 2, 3 и 4.
"""

def Execute(cmd_and_args):
    global runsCount

    sumExecTime = 0
    execTimes = []
    for runIndex in range(0, runsCount):

        p = Popen(cmd_and_args, stdin=PIPE, stdout=PIPE, stderr=PIPE)
        output, err = p.communicate()

        output = output.decode("utf-8")
        err = err.decode("utf-8")

        print(output)
        rc = p.returncode
        if (rc != 0):
            print(f"Exit code = {rc}")
            print(err)

        logFile = ""
        with open("log.txt", "r") as file:
            logFile = file.read()
        print(logFile)

        execTimeStartIndex = logFile.find("Execution time = ") + len("Execution time = ")
        execTimeStopIndex = logFile.find(' ', execTimeStartIndex)

        execTime = float(logFile[execTimeStartIndex:execTimeStopIndex])
        sumExecTime += execTime
        execTimes.append(execTime)
    average = sumExecTime / runsCount
    
    print("Execution times:")
    for st in range(0, len(execTimes)):
        print(f"{st + 1}. {execTimes[st]}")
    print(f"Average = {average}")
    return average

def Main():
    global runsCount
    
    if (len(sys.argv) == 1):
        print(args_message)
        return
    
    cores = []
    
    confsCount = int(sys.argv[1])
    for st in range(confsCount):
        cores.append(int(sys.argv[2 + st]))
    sysArgvIndex = 2 + confsCount
    runsCount = int(sys.argv[sysArgvIndex])
    sysArgvIndex += 1
    cmds = sys.argv[sysArgvIndex:]

    times = []

    for cpuCores in cores:
        cmd = ["mpirun", "-np", str(cpuCores)] + cmds
        time = Execute(cmd)
        times.append(time)

    cores1 = cores[1:]
    speedup = []
    theorySpeedup = []
    efficiency = []
    for cpuCores in cores1:
        Sp = times[0] / times[cpuCores - 1]
        speedup.append(Sp)
        theorySpeedup.append(cpuCores)
        E = Sp / cpuCores * 100
        efficiency.append(E)

    if (not os.path.exists("perf")):
        os.mkdir("perf")

    plt.title("Время исполнения")
    plt.xlabel("Число процессоров")
    plt.ylabel("Время исполнения, сек")
    plt.plot(cores, times, marker='.')
    plt.grid()
    plt.savefig("perf/exec_time.png")
    plt.show()

    plt.title("Ускорение")
    plt.xlabel("Число процессоров")
    plt.ylabel("Ускорение Sp")
    plt.plot(cores1, speedup, marker='.', label = "измер.")
    plt.plot(cores1, theorySpeedup, marker='.', label = "теор.")
    plt.grid()
    plt.legend()
    plt.savefig("perf/speedup.png")
    plt.show()

    plt.title("Эффективность")
    plt.xlabel("Число процессоров")
    plt.ylabel(r"Эффективность $E = Sp/p$")
    plt.plot(cores1, efficiency, marker='.')
    plt.grid()
    plt.savefig("perf/efficiency.png")
    plt.show()


Main()