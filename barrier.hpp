#include <mutex>
#include <condition_variable>

class Barrier
{
private:
  size_t threads;
  unsigned int count;
  std::mutex mutex;
  std::condition_variable cond;

public:
  size_t size(void);
  size_t size(size_t size);
  void wait(void);
};
