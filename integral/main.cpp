#include <cassert>
#include <cmath>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <pthread.h>
#include <semaphore.h>
#include <chrono>

const size_t InitialTasksSize = 6000;

const bool DEBUG = false;

struct Task
{
    double x1;
    double x2;
    double f1;
    double f2;
    double Int;
};

struct TConfig
{
    std::vector<Task> Tasks;
    size_t ProcNumber;
    size_t TasksDone;
    size_t MaxTasksCount;
    double Result;
};

struct GConfig
{
    size_t ThreadsNumber;
    size_t ProcNumber;
    size_t TaskToDoPacketSize;
    size_t ActiveThreads;
    double Eps;

    double Result;
    size_t TotalTasksDone;

    std::vector<Task> Tasks;

    sem_t WaitTasks;
    sem_t GConfAccess;
    pthread_barrier_t SyncExit;
};

double IntFunct(double x)
{
    //return (sin(1/x)*sin(1/x))/(x*x);
    return sin(1/x)/x;
}

void ComputeIntTrapezoid(Task& task)
{
    task.Int = (task.f2 + task.f1)/2 * (task.x2 - task.x1);
}

void DoTasks(TConfig& tconf, GConfig& gconf)
{
    if (tconf.Tasks.size() == 0)
        return;

    Task task = tconf.Tasks.back();
    tconf.Tasks.pop_back();

    for (size_t taskIndex = 0; taskIndex < gconf.TaskToDoPacketSize; taskIndex++)
    {
        double xc = (task.x2 + task.x1) / 2;

        if (xc == task.x2 || xc == task.x1)
        {
            tconf.Result += task.Int;
            
            if (tconf.Tasks.size() == 0)
                return;
                
            task = tconf.Tasks.back();
            tconf.Tasks.pop_back();
            continue;
        }

        double fc = IntFunct(xc);

        Task t1 = Task
        {
            .x1  = task.x1,
            .x2  = xc,
            .f1  = task.f1,
            .f2  = fc
        };
        ComputeIntTrapezoid(t1);

        Task t2 = Task
        {
            .x1  = xc,
            .x2  = task.x2,
            .f1  = fc,
            .f2  = task.f2
        };
        ComputeIntTrapezoid(t2);

        double Ih = task.Int;
        double Ih2 = t1.Int + t2.Int;

        tconf.TasksDone++;

        if (std::abs(Ih - Ih2) < gconf.Eps)
        {
            // Good precise.
            tconf.Result += Ih + Ih2;

            if (tconf.Tasks.size() == 0)
                return;
                
            task = tconf.Tasks.back();
            tconf.Tasks.pop_back();
            continue;
        }
        else
        {
            // Not enough precise.
            tconf.Tasks.push_back(t1);
            task = t2;
            continue;
        }
    }

    tconf.Tasks.push_back(task);
}

static size_t StartSendIndex(size_t size, const GConfig* const gconf)
{
    return size / gconf->ThreadsNumber;
}

static size_t StartRecvIndex(size_t size, const GConfig* const gconf)
{
    return size / gconf->ThreadsNumber;
}

void* threadFunction(void* args)
{
    assert(args);

    GConfig* const gconf = reinterpret_cast<GConfig* const>(args);
    TConfig tconf = {};
    tconf.Tasks.reserve(InitialTasksSize);

    sem_wait(&gconf->GConfAccess);

    tconf.ProcNumber = gconf->ProcNumber++;

    std::cout << "THREAD[" << tconf.ProcNumber << "] inited." << std::endl;

    sem_post(&gconf->GConfAccess);

    while (true)
    {
    get_tasks:
        sem_wait(&gconf->GConfAccess);

        const size_t gtasksCount = gconf->Tasks.size();
        if (gtasksCount != 0)
        {
            size_t tconfSize = tconf.Tasks.size();
            size_t startIndex = StartRecvIndex(gtasksCount, gconf);
            if (startIndex < 5)
                startIndex = 0;
            const auto& startRecvIter = gconf->Tasks.begin() + startIndex;

            tconf.Tasks.insert(tconf.Tasks.end(), startRecvIter, gconf->Tasks.end());
            gconf->Tasks.erase(startRecvIter, gconf->Tasks.end());

            if (DEBUG)
            {
                std::cout << "THREAD[" << tconf.ProcNumber << "] read " 
                        << tconf.Tasks.size() - tconfSize << " tasks." 
                        << "\n\t" << StartRecvIndex(gtasksCount, gconf)
                        << "\n\t" << gtasksCount
                        << "\n\t" << gconf->ThreadsNumber
                        << "\n\t" << gtasksCount / gconf->ThreadsNumber
                        << "\n\t" << tconf.Tasks.size()
                        << "\n\t" << tconfSize
                        << "\n\t" << gconf->Tasks.size()
                        << std::endl;
            }

            gconf->ActiveThreads++;
        }

        sem_post(&gconf->GConfAccess);

        if (gtasksCount == 0)
        {
            sem_wait(&gconf->GConfAccess);
            
            if (gconf->ActiveThreads == 0 && gconf->Tasks.size() == 0)
                goto exit;

            sem_post(&gconf->GConfAccess);

            if (DEBUG)
                std::cout << "THREAD[" << tconf.ProcNumber << "] is waiting..." << std::endl;

            sem_wait(&gconf->WaitTasks);

            sem_wait(&gconf->GConfAccess);
            
            if (gconf->ActiveThreads == 0 && gconf->Tasks.size() == 0)
                goto exit;

            sem_post(&gconf->GConfAccess);
            
            goto get_tasks;
        }
        
        while (tconf.Tasks.size() > 0)
        {
            DoTasks(tconf, *gconf);

            if (tconf.MaxTasksCount < tconf.Tasks.size())
                tconf.MaxTasksCount = tconf.Tasks.size();

            sem_wait(&gconf->GConfAccess);

            if (DEBUG)
                std::cout << "THREAD[" << tconf.ProcNumber << "] done " << tconf.TasksDone << " tasks." << std::endl;

            if (gconf->Tasks.size() == 0 && 
                tconf.Tasks.size() > 0 &&
                gconf->ActiveThreads != gconf->ThreadsNumber &&
                tconf.Tasks.size() > 5)
            {
                size_t tasksCount = tconf.Tasks.size();
                
                const auto& startSendIter = tconf.Tasks.begin() + StartSendIndex(tasksCount, gconf);
                
                gconf->Tasks.insert(gconf->Tasks.end(), startSendIter, tconf.Tasks.end());
                tconf.Tasks.erase(startSendIter, tconf.Tasks.end());

                if (DEBUG)
                {
                    std::cout << "THREAD[" << tconf.ProcNumber << "] send " 
                              << tconf.Tasks.size() - tasksCount << " tasks." << std::endl;
                }

                sem_post(&gconf->WaitTasks);
            }

            sem_post(&gconf->GConfAccess);
        }

        sem_wait(&gconf->GConfAccess);
        
        gconf->ActiveThreads--;

        sem_post(&gconf->GConfAccess);
    }

exit:
    std::cout << "Tasks are done. Terminating..." << std::endl;
    for (size_t st = 0; st < gconf->ThreadsNumber; st++)
        sem_post(&gconf->WaitTasks);

    sem_post(&gconf->GConfAccess);

    pthread_barrier_wait(&gconf->SyncExit);

    sem_wait(&gconf->GConfAccess);
    
    std::cout << "THREAD[" << tconf.ProcNumber << "]:\n"
              << "\tTasks done      = " << tconf.TasksDone << "\n"
              << "\tMax tasks count = " << tconf.MaxTasksCount << "\n" 
              << std::endl;
    
    gconf->Result += tconf.Result;
    gconf->TotalTasksDone += tconf.TasksDone;

    sem_post(&gconf->GConfAccess);

    return 0;
}

int main(int argc, char* argv[])
{
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;

    auto startTime = high_resolution_clock::now();

    if (argc != 7)
    {
        std::cout
            << "Enter as the first argument number of threads.\n"
            << "As the second argument enter start integration limit.\n"
            << "As the third argument enter stop integration limit.\n"
            << "As the fourth argument enter epsilon.\n"
            << "As the fifth argument enter start number of integration intervals.\n"
            << "As the sixth argument enter tasks packet size.\n"
            << std::endl;
        return EXIT_FAILURE;
    }

    size_t threadsNumber = atoi(argv[1]);
    double startInt = atof(argv[2]);
    double stopInt = atof(argv[3]);
    double eps = atof(argv[4]);
    size_t startIntervalsCount = atoi(argv[5]);
    size_t taskPacketSize = atoi(argv[6]);

    GConfig gconf = {};
    gconf.ThreadsNumber = threadsNumber;
    gconf.ActiveThreads = 0;
    gconf.Eps = eps;
    gconf.TaskToDoPacketSize = taskPacketSize;
    gconf.Tasks.reserve(InitialTasksSize);

    double dx = (stopInt - startInt) / static_cast<double>(startIntervalsCount);
    double x = startInt;
    for (size_t st = 0; st < startIntervalsCount; st++)
    {
        Task t =
        {
            .x1 = x,
            .x2 = x + dx,
            .f1 = IntFunct(x),
            .f2 = IntFunct(x + dx)
        };
        ComputeIntTrapezoid(t);

        gconf.Tasks.push_back(t);
        x += dx;
    }

    pthread_t threads[threadsNumber] = {};

    if (sem_init(&gconf.GConfAccess, 0, 1))
        return EXIT_FAILURE;
    if (sem_init(&gconf.WaitTasks, 0, 0))
        return EXIT_FAILURE;
    if (pthread_barrier_init(&gconf.SyncExit, nullptr, gconf.ThreadsNumber))
        return EXIT_FAILURE;

    for (size_t st = 0; st < threadsNumber; st++)
    {
        if (pthread_create(threads + st, nullptr, threadFunction, &gconf))
            return EXIT_FAILURE;
    }

    for (size_t st = 0; st < threadsNumber; st++)
    {
        if (pthread_join(threads[st], nullptr))
            return EXIT_FAILURE;
    }

    sem_destroy(&gconf.GConfAccess);
    sem_destroy(&gconf.WaitTasks);
    pthread_barrier_destroy(&gconf.SyncExit);

    std::cout << std::setprecision(12);

    auto stopTime = high_resolution_clock::now();
    duration<double, std::milli> execTime = stopTime - startTime;

    std::cout << "MAIN THREAD:\n"
              << "\tTasks done     = " << gconf.TotalTasksDone << "\n"
              << "\tResult         = " << gconf.Result << "\n" 
              << "\tExecution time = " << execTime.count() << " ms\n"
              << std::endl;

    std::ofstream file;
    file.open("log.txt", std::ios::out | std::ios::trunc);
    file << "Execution time = " << execTime.count() << " ms" << std::endl;
    file.close();

    return 0;
}