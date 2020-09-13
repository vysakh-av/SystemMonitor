#ifndef PROCESSOR_H
#define PROCESSOR_H

class Processor {
 public:
  float Utilization();  // TODO: See src/processor.cpp

 private:
  float idle_ = 0.0f;
  float nonIdle_ = 0.0f;
  float total_ = 0.0f;
  float cpuPrecentage_ = 0.0f;

  float prevIdle_ = 0.0f;
  float prevNoneIdle_ = 0.0f;
  float prevTotal_ = 0.0f;
};

#endif