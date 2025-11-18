#include <stdio.h>
#include <stdlib.h>

#define N_POINTS 5

typedef struct {
    double x, y, z;
} Point3D;

void getInputPoints(int n_points, Point3D* points);
int findMinX(int n_points, const Point3D* points);
int findMaxY(int n_points, const Point3D* points);
void outputPoint(const char* label, Point3D point);

int main() {
    Point3D points[N_POINTS];

    getInputPoints(N_POINTS, points);

    int minXPointIndex = findMinX(N_POINTS, points);
    int maxYPointIndex = findMaxY(N_POINTS, points);

    outputPoint("Min X point:", points[minXPointIndex]);
    outputPoint("Max Y point:", points[maxYPointIndex]);

    return EXIT_SUCCESS;
}

void getInputPoints(int n_points, Point3D* points) {
    for (int i = 0; i < n_points; i++) {
        
    }
}

int findMinXPoint(int n_points, const Point3D* points) {
    int minIndex = 0;
    for (int i = 1; i <= n_points; i++) {
        if (points[i].x < points[minIndex].x) {
            minIndex = i;
        }
    }

    return minIndex;
}

int findMaxYPoint(int n_points, const Point3D* points) {
    int maxIndex = 0;
    for (int i = 1; i <= n_points; i++) {
        if (points[i].x > points[maxIndex].x) {
            maxIndex = i;
        }
    }

    return maxIndex;
}

void outputPoint(const char* label, Point3D point) {
    printf("%s (x: %.3f, y: %.3f, z: %.3f)\n", label, point.x, point.y, point.z);
}