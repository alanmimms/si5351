#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>

void find_best_fraction(double f, unsigned int *best_b, unsigned int *best_c) {
  unsigned int max_val = 1048575;
  double min_error = DBL_MAX;

  *best_b = 0;
  *best_c = 1;

  for (unsigned int c = 1; c <= max_val; ++c) {
    double desired_b_double = f * c;
    unsigned int current_b = (unsigned int)round(desired_b_double);

    if (current_b <= max_val) {
      double current_error = fabs(f - (double)current_b / c);
      if (current_error < min_error) {
	min_error = current_error;
	*best_b = current_b;
	*best_c = c;
      }
    }
  }
}

int main() {
  double crystal_freq = 25000000.0; // 25MHz
  unsigned int max_frac_val = 1048575;

  // Parameters for the Feedback MultiSynth (PLL)
  unsigned int a_fb, b_fb, c_fb;

  // Parameters for the Output MultiSynth
  unsigned int a_out, b_out, c_out;
  double output_ratio_f;

  printf("Enter the integer part (a), numerator (b), and denominator (c) for the Feedback MultiSynth divider (separated by spaces): ");
  if (scanf("%u %u %u", &a_fb, &b_fb, &c_fb) != 3) {
    fprintf(stderr, "Invalid input for Feedback MultiSynth.\n");
    return 1;
  }
  if (c_fb == 0 || c_fb > max_frac_val) {
    fprintf(stderr, "Denominator c_fb must be between 1 and %u.\n", max_frac_val);
    return 1;
  }
  if (b_fb >= c_fb) {
    fprintf(stderr, "Numerator b_fb must be less than denominator c_fb.\n");
    return 1;
  }

  double feedback_ratio = (double)a_fb + (double)b_fb / (double)c_fb;
  if (feedback_ratio < 15.0 || feedback_ratio > 90.0) {
    fprintf(stderr, "Feedback MultiSynth ratio (a + b/c = %.9f) must be between 15 and 90.\n", feedback_ratio);
    return 1;
  }

  printf("Enter the desired floating point division ratio for the Output MultiSynth: ");
  if (scanf("%lf", &output_ratio_f) != 1) {
    fprintf(stderr, "Invalid input for Output MultiSynth ratio.\n");
    return 1;
  }

  a_out = (unsigned int)floor(output_ratio_f);
  double fractional_part = output_ratio_f - (double)a_out;

  find_best_fraction(fractional_part, &b_out, &c_out);

  double output_ratio = (double)a_out + (double)b_out / (double)c_out;
  if (!((output_ratio == 4) || (output_ratio == 6) || (output_ratio == 8) || (output_ratio >= 8.0 + 1.0 / max_frac_val && output_ratio <= 2048.0))) {
    fprintf(stderr, "Output MultiSynth ratio (a + b/c = %.9f) must be 4, 6, 8, or between approximately 8.0 and 2048.0.\n", output_ratio);
    return 1;
  }
  if (c_out == 0 || c_out > max_frac_val) {
    fprintf(stderr, "Denominator c_out must be between 1 and %u.\n", max_frac_val);
    return 1;
  }
  if (b_out >= c_out) {
    fprintf(stderr, "Numerator b_out must be less than denominator c_out.\n");
    return 1;
  }

  unsigned rDiv = 0;
  printf("Enter R to divide by (1..128): ");
  if (scanf("%u", &rDiv) != 1) {
    fprintf(stderr, "Bad R value (must be 1..128) %u\n", rDiv);
    return 1;
  }

  // Calculate the Feedback MultiSynth division ratio
  double feedback_multisynth_ratio = feedback_ratio;

  // Calculate the VCO frequency
  double vco_freq = crystal_freq * feedback_multisynth_ratio;

  // Calculate the Output MultiSynth division ratio
  double output_multisynth_ratio = output_ratio;

  // Calculate the final output frequency
  double output_freq = vco_freq / output_multisynth_ratio / (double) rDiv;

  printf("\nCrystal Frequency: %.6f Hz\n", crystal_freq);
  printf("Feedback MultiSynth Divider: %u + %u/%u = %.9f\n", a_fb, b_fb, c_fb, feedback_multisynth_ratio);
  printf("VCO Frequency: %.6f Hz\n", vco_freq);
  printf("Output MultiSynth Divider: %u + %u/%u = %u + %u/%u = %.9f\n", a_out, b_out, c_out, a_out, b_out, c_out, output_multisynth_ratio);
  printf("Output Frequency: %.6f Hz\n", output_freq);

  return 0;
}
