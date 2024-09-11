This folder contains some artifacts of Vishal Canumalla's
project at Adobe Research, prototyping a simple
metric accumulation analysis tool.

There are two examples in this folder.

1. the `tutorial_sca_example.cpp` which is a pipeline that 
closely mirrors the pipeline demoed in `lesson_08_scheduling`.
To test SCA on this example, compile the pipeline using
cmake, g++, etc. and run the output. A static library should
be generated, which relies on `metrics.cpp` in this folder,
which contains some simple accumulation functions to a global variable.

You can then run a driver in `demo/tutorial/driver.cpp`
to see a way of collecting the metrics. See `demo/tutorial/Makefile`
for the full compilation flow (its likely broken as I've moved
stuff around and changed filenames for packaging).

2. The `hist_generator.cpp` which is a straight rip of the
`apps/hist` generator. After compiling the generator, run
with a target that includes the flag `sca_metrics`. For example,
I ran
```
./hist.generator -g hist -o tmp target=arm-64-osx-arm_dot_prod-arm_fp16-c_plus_plus_name_mangling-no_asserts-sca_metrics
```

There is a similar driver metric collection function in `demo/hist`, which also relies on
linking the generator static library, and the metrics accumulation.

There are also some additional (very brittle) python scripts
which generate some graphs based off the stdout generated
from the metric drivers. Use the `print_serialized` functions
rather than the `print_display` functions if you'd 
like to use these python scripts.