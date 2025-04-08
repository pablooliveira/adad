# Adad

Adad is a small tool for profiling the energy consumption of specific functions
in x86-64 binaries.  Other tools measure the energy consumption of an entire
application, which can be difficult to interpret when there are long
initialization phases or when the function of interest is only a small part of
the application.

Adad focuses specifically on measuring efficiently a single function. To achieve this goal
with low overhead it relies on,

* [e9patch](https://github.com/GJDuck/e9patch) to instrument the target function;
* the `perf_event` kernel interface to collect RAPL samples.

## Installation

To install adad, clone the repository and run the following command:

```bash
./build.sh
```

## Caveat and Limitations

Adad requires permissions to read system-wide performance counters. Please
ensure that perf-event paranoid permissions are set to -1 with:

```bash
sudo sysctl -w kernel.perf_event_paranoid=-1
```

RAPL domains are only available system-wide and cannot be collected for a
single individual process. Therefore:

* Ensure that there are as few as possible background processes running on your measurement environment ;

* For multi-threaded programs only the main thread will call the adad runtime measurement hooks.
  But all the threads will contribute to RAPL measures. Please be very careful when interpreting the results.

## Usage

First, run the command:

```bash
./adad-patch <binary> <symbol>
```

This will instrument the function <symbol> with calls to the adad runtime.
By default the RAPL domain energy-pkg is used. But you can specify the domain by setting the environment variable `ADAD_RAPL_DOMAIN`.
For example,

```bash
export ADAD_RAPL_DOMAIN=energy-cores
```

Once the instrumented binary has been run, you can use the command:

```bash
./adad-report
```

to print the statistics of the RAPL samples collected.

Example:

```bash
./adad-patch ./my_binary my_function
./my_binary
./adad-report
energy-pkg mean: 3.5981 ± 0.0666 (1.85%) Joules from 2 samples
```