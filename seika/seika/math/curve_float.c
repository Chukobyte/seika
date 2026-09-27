#include "curve_float.h"

#include <float.h>
#include "seika/math/math.h"
#include "seika/assert.h"

// Helper functions
void control_point_swap(SkaCurveControlPoint* point_a, SkaCurveControlPoint* point_b) {
    SkaCurveControlPoint temp = *point_a;
    *point_a = *point_b;
    *point_b = temp;
}

// Function to perform Selection Sort
void selection_sort_curve_float(SkaCurveFloat* curve) {
    usize min_x;
    // One by one move boundary of unsorted subarray
    for (usize i = 0; i < curve->control_point_count - 1; i++) {
        // Find the minimum element in unsorted array
        min_x = i;
        for (usize j = i + 1; j < curve->control_point_count; j++) {
            if (curve->control_points[j].x < curve->control_points[min_x].x) {
                min_x = j;
            }
        }
        // Swap the found minimum element
        // with the first element
        control_point_swap(&curve->control_points[min_x], &curve->control_points[i]);
    }
}

//--- SkaCurveFloat ---//
void ska_curve_float_add_control_point(SkaCurveFloat* curve, SkaCurveControlPoint point) {
    SKA_ASSERT_FMT(curve->control_point_count + 1 < SKA_CURVE_MAX_CONTROL_POINTS, "Trying to add more points than max '%u'", SKA_CURVE_MAX_CONTROL_POINTS);
    curve->control_points[curve->control_point_count++] = point;
    selection_sort_curve_float(curve);
}

void ska_curve_float_add_control_points(SkaCurveFloat* curve, SkaCurveControlPoint points[], usize count) {
    if (count == 0) {
        return;
    }
    for (usize i = 0; i < count; i++) {
        SKA_ASSERT_FMT(curve->control_point_count + 1 < SKA_CURVE_MAX_CONTROL_POINTS, "Trying to add multiple points that go beyond the max of '%u'", SKA_CURVE_MAX_CONTROL_POINTS);
        curve->control_points[curve->control_point_count++] = points[i];
    }
    selection_sort_curve_float(curve);
}

bool ska_curve_float_remove_control_point(SkaCurveFloat* curve, f64 x, f64 y) {
    for (usize i = 0; i < curve->control_point_count; i++) {
        SkaCurveControlPoint* point = &curve->control_points[i];
        if (ska_math_is_almost_equal_double_default(x, point->x) && ska_math_is_almost_equal_double_default(y, point->y)) {
            // We've found a matching point so set it's x to the highest value, sort it, then decrement the point count.
            point->x = DBL_MAX;
            selection_sort_curve_float(curve);
            curve->control_point_count--;
            return true;
        }
    }
    return false;
}

f64 ska_curve_float_eval(const SkaCurveFloat* curve, f64 t) {
    if (curve->control_point_count == 0) {
        return 0.0;
    } else if (curve->control_point_count == 1) {
        return curve->control_points[0].y;
    }
    usize left_index = 0;
    usize right_index = curve->control_point_count - 1;
    while (right_index - left_index > 1) {
        usize mid_index = (left_index + right_index) / 2;
        if (t < curve->control_points[mid_index].x) {
            right_index = mid_index;
        } else {
            left_index = mid_index;
        }
    }

    const SkaCurveControlPoint* left_point = &curve->control_points[left_index];
    const SkaCurveControlPoint* right_point = &curve->control_points[right_index];

    const f64 t1 = (t - left_point->x) / (right_point->x - left_point->x);
    const f64 t2 = t1 * t1;
    const f64 t3 = t2 * t1;

    const f64 a = 2.0f * t3 - 3.0f * t2 + 1.0f;
    const f64 b = -2.0f * t3 + 3.0f * t2;
    const f64 c = t3 - 2.0f * t2 + t1;
    const f64 d = t3 - t2;

    return a * left_point->y + b * right_point->y + c * left_point->tangent_out + d * right_point->tangent_in;
}
