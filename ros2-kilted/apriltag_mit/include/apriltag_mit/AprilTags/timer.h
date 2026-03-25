#ifndef APRILTAGS_TIMER_H_
#define APRILTAGS_TIMER_H_

#include <chrono>
#include <string>
#include <cassert>
#include <ostream>
#include <iomanip>
#include <iostream>
#include <thread>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>

namespace AprilTags {

// Useful typedefs
using sec = std::chrono::seconds;
using ms = std::chrono::milliseconds;
using us = std::chrono::microseconds;
using ns = std::chrono::nanoseconds;

// SIFANE!
template <typename>
struct is_duration : std::false_type {};

template <typename T, typename U>
struct is_duration<std::chrono::duration<T, U>> : std::true_type {};

// Units
template <typename T>
std::string durationUnit() {
  return "unknown unit";
}

#define TIMER_UNIT(time)             \
  template <>                        \
  std::string durationUnit<time>() { \
    return #time;                    \
  }

TIMER_UNIT(sec)
TIMER_UNIT(ms)
TIMER_UNIT(us)
TIMER_UNIT(ns)

#undef TIMER_UNIT

template <typename NumT, typename DenT>
double ratio() {
  typedef typename NumT::period NumPeriod;
  typedef typename DenT::period DenPeriod;
  typedef typename std::ratio_divide<NumPeriod, DenPeriod>::type RatioType;
  return static_cast<double>(RatioType::num) / RatioType::den;
}

namespace bac = boost::accumulators;

/**
 * @brief The Timer class, a minimum timer class
 */
template <typename DurationT,
          typename ClockT = std::chrono::high_resolution_clock>
class Timer {
  static_assert(is_duration<DurationT>::value, "Not a valid duration type");

 public:
  explicit Timer(const std::string& name, bool start_now = true) : name_(name) {
    if (start_now) start();
  }

  const std::string& name() const { return name_; }

  /**
   * @brief Start, start timer
   */
  void start() {
    assert(!timing_);
    timing_ = true;
    start_ = ClockT::now();
  }

  /**
   * @brief Stop, stop timer
   */
  void stop() {
    elapsed_ = std::chrono::duration_cast<DurationT>(ClockT::now() - start_);
    assert(timing_);
    timing_ = false;
    acc_(elapsed());  // Update accumulator
  }

  /**
   * @brief Reset
   */
  void reset() {
    timing_ = false;
    elapsed_ = DurationT(0);
    acc_ = bac::accumulator_set<double, AccumulatorFeatures>();
  }

  /**
   * @brief Sleep
   * @param tick
   */
  template <typename T = DurationT>
  void sleep(int tick) {
    std::this_thread::sleep_for(T(tick));
  }

  /**
   * @brief Elapsed
   * @return
   */
  template <typename T = DurationT>
  double elapsed() const {
    return elapsed_.count() * ratio<DurationT, T>();
  }

  /**
   * @brief elapsedStr
   * @return
   */
  template <typename T = DurationT>
  std::string elapsedStr() const {
    return std::to_string(elapsed<T>()) + unitStr<T>();
  }

  /**
   * @brief ElapsedDuration
   * @return
   */
  template <typename T = DurationT>
  T elapsedDuration() const {
    return std::chrono::duration_cast<T>(elapsed_);
  }

  /**
   * @brief UnitStr
   * @return
   */
  template <typename T = DurationT>
  std::string unitStr() const {
    return durationUnit<T>();
  }

  /**
   * @brief BaseUnitStr
   * @return
   */
  std::string baseUnitStr() const { return unitStr(); }

  /**
   * @brief Mean
   */
  template <typename T = DurationT>
  double mean() const {
    return bac::extract_result<bac::tag::mean>(acc_) * ratio<DurationT, T>();
  }

  /**
   * @brief Max
   */
  template <typename T = DurationT>
  double max() const {
    return bac::extract_result<bac::tag::max>(acc_) * ratio<DurationT, T>();
  }

  /**
   * @brief Min
   */
  template <typename T = DurationT>
  double min() const {
    return bac::extract_result<bac::tag::min>(acc_) * ratio<DurationT, T>();
  }

  /**
   * @brief Sum
   */
  template <typename T = DurationT>
  double sum() const {
    return bac::extract_result<bac::tag::sum>(acc_) * ratio<DurationT, T>();
  }

  /**
   * @brief count
   */
  size_t count() const { return bac::extract_result<bac::tag::count>(acc_); }

  template <typename T = DurationT>
  void report(std::ostream& os = std::cout) {
    os << "[" << name() << "] " << elapsedStr<T>() << std::endl;
  }

  /**
   * @brief statsStr
   */
  template <typename T = DurationT>
  std::string statsStr() {
    return std::string("count: ") + std::to_string(count()) + ", mean: " +
           std::to_string(mean<T>()) + ", min: " + std::to_string(min<T>()) +
           ", max: " + std::to_string(max<T>()) + ", unit: " + unitStr<T>();
  }

  /**
   * @brief reportStats Print stats to os
   */
  template <typename T = DurationT>
  void reportStats(std::ostream& os = std::cout) {
    os << "[" << name() << "] " << statsStr<T>() << std::endl;
  }

 private:
  using AccumulatorFeatures =
      bac::features<bac::tag::sum, bac::tag::min, bac::tag::max, bac::tag::mean,
                    bac::tag::count>;
  std::string name_{"timer"};
  bool timing_{false};
  DurationT elapsed_{0};
  typename ClockT::time_point start_;
  bac::accumulator_set<double, AccumulatorFeatures> acc_;
};

// More useful typedefs
using TimerSec = Timer<sec>;
using TimerMs = Timer<ms>;
using TimerUs = Timer<us>;
using TimerNs = Timer<ns>;

/// Helper function
template <typename ClockType>
void printClockData() {
  std::cout << "- precision: ";
  // If time unit is less or equal one millisecond
  typedef typename ClockType::period RatioType;
  if (std::ratio_less_equal<RatioType, std::milli>::value) {
    // Convert to and print as milliseconds
    typedef typename std::ratio_multiply<RatioType, std::kilo>::type TT;
    std::cout << std::fixed << static_cast<double>(TT::num) / TT::den << " ms"
              << std::endl;
  } else {
    // Print as seconds
    std::cout << std::fixed
              << static_cast<double>(RatioType::num) / RatioType::den << " sec"
              << std::endl;
  }
  std::cout << "- is_steady: " << std::boolalpha << ClockType::is_steady
            << std::endl;
}

template <typename DurationT,
          typename ClockT = std::chrono::high_resolution_clock>
class ScopedTimer {
 public:
  ScopedTimer(const std::string& name) : timer_(name) {}
  ~ScopedTimer() {
    timer_.stop();
    timer_.report();
  }

 private:
  Timer<DurationT, ClockT> timer_;
};

using ScopedTimerSec = ScopedTimer<sec>;
using ScopedTimerMs = ScopedTimer<ms>;
using ScopedTimerUs = ScopedTimer<us>;
using ScopedTimerNs = ScopedTimer<ns>;

}  // namespace AprilTags

#endif
