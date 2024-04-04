#include "curve_float.h"

#include "seika/math/math.h"
#include "seika/assert.h"

// Helper functions
void control_point_swap(SkaCurveControlPoint* pointA, SkaCurveControlPoint* pointB) {
    SkaCurveControlPoint temp = *pointA;
    *pointA = *pointB;
    *pointB = temp;
}

// Function to perform Selection Sort
void selection_sort_curve_float(SkaCurveFloat* curve) {
    usize minX;
    // One by one move boundary of unsorted subarray
    for (usize i = 0; i < curve->controlPointCount - 1; i++) {
        // Find the minimum element in unsorted array
        minX = i;
        for (usize j = i + 1; j < curve->controlPointCount; j++) {
            if (curve->controlPoints[j].x < curve->controlPoints[minX].x) {
                minX = j;
            }
        }
        // Swap the found minimum element
        // with the first element
        control_point_swap(&curve->controlPoints[minX], &curve->controlPoints[i]);
    }
}

//--- SkaCurveFloat ---//
void ska_curve_float_add_control_point(SkaCurveFloat* curve, SkaCurveControlPoint point) {
    SKA_ASSERT_FMT(curve->controlPointCount + 1 < SKA_CURVE_MAX_CONTROL_POINTS, "Trying to add more points than max '%u'", SKA_CURVE_MAX_CONTROL_POINTS);
    curve->controlPoints[curve->controlPointCount++] = point;
    selection_sort_curve_float(curve);
}

void ska_curve_float_add_control_points(SkaCurveFloat* curve, SkaCurveControlPoint points[], usize count) {
    if (count == 0) {
        return;
    }
    for (usize i = 0; i < count; i++) {
        SKA_ASSERT_FMT(curve->controlPointCount + 1 < SKA_CURVE_MAX_CONTROL_POINTS, "Trying to add multiple points that go beyond the max of '%u'", SKA_CURVE_MAX_CONTROL_POINTS);
        curve->controlPoints[curve->controlPointCount++] = points[i];
    }
    selection_sort_curve_float(curve);
}

bool ska_curve_float_remove_control_point(SkaCurveFloat* curve, f64 x, f64 y) {
    for (usize i = 0; i < curve->controlPointCount; i++) {
        SkaCurveControlPoint* point = &curve->controlPoints[i];
        if (ska_math_is_almost_equal_double_default(x, point->x) && ska_math_is_almost_equal_double_default(y, point->y)) {
            // We've found a matching point so set it's x to the highest value, sort it, then decrement the point count.
            point->x = DBL_MAX;
            selection_sort_curve_float(curve);
            curve->controlPointCount--;
            return true;
        }
    }
    return false;
}

f64 ska_curve_float_eval(const SkaCurveFloat* curve, f64 t) {
    if (curve->controlPointCount == 0) {
        return 0.0;
    } else if (curve->controlPointCount == 1) {
        return curve->controlPoints[0].y;
    }
    usize leftIndex = 0;
    usize rightIndex = curve->controlPointCount - 1;
    while (rightIndex - leftIndex > 1) {
        usize midIndex = (leftIndex + rightIndex) / 2;
        if (t < curve->controlPoints[midIndex].x) {
            rightIndex = midIndex;
        } else {
            leftIndex = midIndex;
        }
    }

    const SkaCurveControlPoint* leftPoint = &curve->controlPoints[leftIndex];
    const SkaCurveControlPoint* rightPoint = &curve->controlPoints[rightIndex];

    const f64 t1 = (t - leftPoint->x) / (rightPoint->x - leftPoint->x);
    const f64 t2 = t1 * t1;
    const f64 t3 = t2 * t1;

    const f64 a = 2.0f * t3 - 3.0f * t2 + 1.0f;
    const f64 b = -2.0f * t3 + 3.0f * t2;
    const f64 c = t3 - 2.0f * t2 + t1;
    const f64 d = t3 - t2;

    return a * leftPoint->y + b * rightPoint->y + c * leftPoint->tangentOut + d * rightPoint->tangentIn;
}
