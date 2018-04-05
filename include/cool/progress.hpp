#ifndef COOL_PROGRESS_HXX_INCLUDED
#define COOL_PROGRESS_HXX_INCLUDED

#include <chrono>
#include <type_traits>

#if __cplusplus >= 201703L
/// \exclude
#define RESULT_OF_T(F, ...) std::invoke_result_t<F, __VA_ARGS__>
#else
/// \exclude
#define RESULT_OF_T(F, ...) typename std::result_of<F(__VA_ARGS__)>::type
#endif

namespace cool
{

template <typename Rep = double, typename Period = std::ratio<1>> class progress
{
  static_assert(std::is_floating_point<Rep>::value, "Rep must be a floating point");
  static_assert(std::is_same<Period, std::ratio<Period::num, Period::den>>::value, "Period must be a std::ratio");

public:
  using duration = std::chrono::duration<Rep, Period>;
  using value_type = Rep;

  explicit progress(value_type progress_threshold = 0.01, duration time_threshold = duration{1})
    : progress_threshold{progress_threshold}, time_threshold{time_threshold}, current_progress{0.0},
      last_update_time{std::chrono::steady_clock::now()}
  {
  }

  template <typename F>
  auto update(value_type perc, const F& f) ->
    typename std::enable_if<std::is_same<RESULT_OF_T(F, value_type, duration), void>::value, bool>::type
  {
    if (perc < current_progress + progress_threshold)
      return false;

    const auto now = std::chrono::steady_clock::now();
    const auto elapsed = std::chrono::duration_cast<duration>(now - last_update_time);
    if (elapsed < time_threshold)
      return false;

    f(perc - current_progress, elapsed);

    current_progress = perc;
    last_update_time = now;

    return true;
  }

private:
  value_type progress_threshold;
  duration time_threshold;

  value_type current_progress;
  std::chrono::steady_clock::time_point last_update_time;
};

} // namespace cool

#undef RESULT_OF_T

#endif // COOL_PROGRESS_HXX_INCLUDED
