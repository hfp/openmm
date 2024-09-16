#include <OpenMM.h>
#include <openmm/opencl/OpenCLPlatform.h>
// #include <openmm/cuda/CudaPlatform.h>

#include <cstdio>
#include <fstream>
#include <sys/time.h>
#include <iostream>
#include <string>
#include <map>

void runBenchmark(
  const std::string &systemFile,
  const std::string &stateFile,
  const double stepSize,
  const std::string &filename,
  const int steps = 10,
  const std::string &platform = "OpenCL",
  const std::string &platformIndex = "0",
  const std::string &deviceIndex = "0")
{
  timeval startBenchmarkTime, endBenchmarkTime;
  gettimeofday(&startBenchmarkTime, NULL);

  std::ofstream ostrm(filename, std::ios::out);
  ostrm << "\nsteps=" << steps << "\n";
  ostrm << "\nBenchmark: " << systemFile << "\n";

  OpenMM::Platform::getPlatformByName(platform);

  std::ifstream systemStream(systemFile.c_str());
  OpenMM::System *system = OpenMM::XmlSerializer::deserialize<OpenMM::System>(systemStream);

  std::ifstream stateStream(stateFile.c_str());
  OpenMM::State *state = OpenMM::XmlSerializer::deserialize<OpenMM::State>(stateStream);

  OpenMM::LangevinMiddleIntegrator integrator(300.0, 91.0, stepSize);

  std::map<std::string, std::string> props;
  if (platformIndex != "0" || deviceIndex != "0") {
    props["OpenCLPlatformIndex"] = platformIndex;
    props["DeviceIndex"] = deviceIndex;
  }

  OpenMM::Context context(*system, integrator, OpenMM::Platform::getPlatformByName(platform), props);

  ostrm << "Using OpenMM platform: " << context.getPlatform().getName() << "\n";

  context.setState(*state);
  integrator.step(1);
  context.getState(OpenMM::State::Forces);

  timeval startTime, endTime;

  gettimeofday(&startTime, NULL);
  integrator.step(steps);
  context.getState(OpenMM::State::Positions);
  gettimeofday(&endTime, NULL);

  double elapsed = (endTime.tv_sec - startTime.tv_sec) + (endTime.tv_usec - startTime.tv_usec) * 1e-6;

  ostrm << "ns/day: " << (0.001 * stepSize * steps * 86400 / elapsed) << "\n";
  ostrm << "simulation time: " << elapsed << " seconds\n";

  gettimeofday(&endBenchmarkTime, NULL);
  elapsed = (endBenchmarkTime.tv_sec - startBenchmarkTime.tv_sec) + (endBenchmarkTime.tv_usec - startBenchmarkTime.tv_usec) * 1e-6;

  ostrm << "simulation time to completion: " << elapsed << " seconds\n";

  delete system;
  delete state;
}

void help(const std::string &message)
{
  std::cout << "\nterminate called after throwing an instance of 'OpenMM::OpenMMException'" << std::endl;
  std::cout << "  what():  " << message << "\n" << std::endl;
  std::cout << "\nRun: ./benchmark [steps=10][platform=OpenCL,CPU,Reference][platformIndex][deviceIndex]" << std::endl;
  std::cout << "E.g.:  ./benchmark 1000" << std::endl;
  std::cout << "E.g.:  ./benchmark 1000  OpenCL  0  0,1" << std::endl;
}

int main(int argc, char *argv[])
{
  OpenMM::Platform::loadPluginsFromDirectory(OpenMM::Platform::getDefaultPluginsDirectory());
  std::string filename{"tmp.log"};
  int steps{10};
  std::string platform{"OpenCL"};
  std::string platformIndex{"0"};
  std::string deviceIndex{"0"};
  switch (argc) {
  case 1:
    break;
  case 2:
    filename = std::string(argv[1]);
    break;
  case 3:
    filename = std::string(argv[1]);
    steps = std::atoi(argv[2]);
    break;
  case 4:
    filename = std::string(argv[1]);
    steps = std::atoi(argv[2]);
    platform = std::string(argv[3]);
    break;
  case 5:
    filename = std::string(argv[1]);
    steps = std::atoi(argv[2]);
    platform = std::string(argv[3]);
    platformIndex = std::string(argv[4]);
    break;
  case 6:
    filename = std::string(argv[1]);
    steps = std::atoi(argv[2]);
    platform = std::string(argv[3]);
    platformIndex = std::string(argv[4]);
    deviceIndex = std::string(argv[5]);
    break;
  }
  try {
    runBenchmark("apoa1rf-system.xml", "apoa1-state.xml", 0.004, filename, steps, platform, platformIndex, deviceIndex);
  }
  catch (std::exception &ex) {
    help(std::string(ex.what()));
  }
  return 0;
}
