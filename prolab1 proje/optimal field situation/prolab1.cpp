#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <graphics.h>
#include <limits.h> 
#include <math.h>

struct Point {
    int x;
    int y;
};

// Processing of downloaded data is performed.
size_t WriteCallback(void *contents, size_t size, size_t nmemb, char **data) {
    size_t realsize = size * nmemb;
    *data = (char *)realloc(*data, realsize + 1);
    if (*data == NULL) {
        fprintf(stderr, "Memory error\n");
        return 0;
    }
    memcpy(*data, contents, realsize);
    (*data)[realsize] = '\0';
    return realsize;
}

void drawShape(struct Point points[], int size1) {
    for (int i = 1; i < size1; ++i) {
        line(points[i - 1].x, points[i - 1].y, points[i].x, points[i].y);
    }
    line(points[size1 - 1].x, points[size1 - 1].y, points[0].x, points[0].y);
}

double calculateShapeArea(struct Point points[], int size, int cellSize) {
    double area = 0.0;
    for (int i = 0; i < size - 1; ++i) {
        area += (points[i].x * points[i + 1].y - points[i + 1].x * points[i].y);
    }
    area += (points[size - 1].x * points[0].y - points[0].x * points[size - 1].y);
    area = abs(area) / 2.0;
    area /= cellSize * cellSize;
    return area;
}

// Checks if the point is inside for the painting and unitSquareCount functions.
int isInside(struct Point points[], int size, float x, float y) {
    int intersectionCount = 0;

    for (int i = 0; i < size; ++i) {
        float x1 = points[i].x;
        float y1 = points[i].y;
        float x2 = points[(i + 1) % size].x;
        float y2 = points[(i + 1) % size].y;

        if ((y1 >= y && y2 < y) || (y2 >= y && y1 < y)) {
            if (x1 == x2 && x1 >= x) {
                intersectionCount++;
            } else if (x1 != x2 && x < (x2 - x1) * (y - y1) / (y2 - y1) + x1) {
                intersectionCount++;
            }
        }
    }
    return (intersectionCount % 2 != 0);
}

void paint(struct Point points[], int size, int cellSize) {
    int minX = INT_MAX, minY = INT_MAX, maxX = INT_MIN, maxY = INT_MIN;

    for (int i = 0; i < size; ++i) {
        if (points[i].x < minX) minX = points[i].x;
        if (points[i].y < minY) minY = points[i].y;
        if (points[i].x > maxX) maxX = points[i].x;
        if (points[i].y > maxY) maxY = points[i].y;
    }

    for (int x = minX; x <= maxX; x++) {
        for (int y = minY; y <= maxY; y++) {
            if (isInside(points, size, x + (cellSize / 2), y + (cellSize / 2))) {
                floodfill(x + (cellSize / 2), y + (cellSize / 2), GREEN);
            }
        }
    }
}

int countUnitSquares(struct Point points[], int size, int cellSize) {
    int minX = INT_MAX, minY = INT_MAX, maxX = INT_MIN, maxY = INT_MIN;
    int unitSquareCount = 0;

    for (int i = 0; i < size; ++i) {
        if (points[i].x < minX) minX = points[i].x;
        if (points[i].y < minY) minY = points[i].y;
        if (points[i].x > maxX) maxX = points[i].x;
        if (points[i].y > maxY) maxY = points[i].y;
    }

    for (int x = minX; x < maxX; x += cellSize) {
        for (int y = minY; y < maxY; y += cellSize) {
            if (isInside(points, size, x + (cellSize / 2), y + (cellSize / 2))) {
                unitSquareCount++;
            }
        }
    }

    return unitSquareCount;
}

int main(void) {
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if (curl) {
        char *data = NULL;
        curl_easy_setopt(curl, CURLOPT_URL, "http://abilgisayar.kocaeli.edu.tr/prolab1/prolab1.txt");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            return 1;
        }
        curl_easy_cleanup(curl);

        int selectedLine;
        int unitDrillCost;
        int unitPlatformCost;
        printf("Select Line Number (1, 2, ...): ");
        scanf("%d", &selectedLine);
        printf("Enter unit drill cost between 1 and 10: ");
        scanf("%d", &unitDrillCost);
        printf("Enter unit platform cost: ");
        scanf("%d", &unitPlatformCost);

        int pointCount = 0;
        struct Point points[200];

        // Window opens and grid is created.
        initwindow(800, 600);
        int cellSize = 10;
        int maxX = getmaxx();
        int maxY = getmaxy();
        setcolor(GREEN);

        for (int x = 0; x <= maxX; x += cellSize) {
            line(x, 0, x, maxY);
        }
        for (int y = 0; y <= maxY; y += cellSize) {
            line(0, y, maxX, y);
        }

        char *token = strtok(data, "\n");
        int lineCount = 0;
        while (token != NULL) {
            lineCount++;
            if (lineCount == selectedLine) {
                char *pointsString = strchr(token, '(');

                while (pointsString != NULL) {
                    int x, y;
                    if (sscanf(pointsString, "(%d,%d)", &x, &y) == 2) {
                        points[pointCount].x = x * cellSize;
                        points[pointCount].y = y * cellSize;
                        pointCount++;
                    }

                    pointsString = strchr(pointsString + 1, '(');
                }

                int *xyPoints = (int *)malloc(2 * pointCount * sizeof(int));
                for (int i = 0; i < pointCount; ++i) {
                    xyPoints[2 * i] = points[i].x;
                    xyPoints[2 * i + 1] = points[i].y;
                }

                for (int i = 0; i < pointCount; ++i) {
                    int j;
                    for (j = i + 1; j < pointCount; ++j) {
                        if (points[i].x == points[j].x && points[i].y == points[j].y) {
                            setcolor(WHITE);
                            drawShape(points + i, j - i + 1);
                            setfillstyle(SOLID_FILL, YELLOW);
                            fillpoly(j - i + 1, xyPoints + 2 * i);
                            double area = calculateShapeArea(points + i, j - i + 1, cellSize);
                            printf("Area of the drawn shape: %.2lf\n", area);
                            printf("Source reserve value: %.2lf\n", area * 10);

                            i = j;
                            break;
                        }
                    }
                    if (j == pointCount) {
                        setcolor(WHITE);
                        drawShape(points + i, 1);
                        setfillstyle(SOLID_FILL, YELLOW);
                        fillpoly(1, xyPoints + 2 * i);
                        double area = calculateShapeArea(points + i, 1, cellSize);
                        printf("Area of the drawn shape: %.2lf\n", area);
                        printf("Source reserve value: %.2lf\n", area * 10);
                    }
                }
                free(xyPoints);
                break;
            }
            token = strtok(NULL, "\n");
        }

        initwindow(800, 600);
        setcolor(GREEN);

        for (int x = 0; x <= maxX; x += cellSize) {
            line(x, 0, x, maxY);
        }
        for (int y = 0; y <= maxY; y += cellSize) {
            line(0, y, maxX, y);
        }

        for (int i = 0; i < pointCount; i++) {
            int j;
            for (j = i + 1; j < pointCount; ++j) {
                if (points[i].x == points[j].x && points[i].y == points[j].y) {
                    paint(points + i, j - i + 1, cellSize);
                    i = j;
                    break;
                }
            }
            if (j == pointCount)
                paint(points + i, 1, cellSize);
        }

        int unitSquareCount = countUnitSquares(points, pointCount, cellSize);
        double area = unitSquareCount;
        printf("Number of unit squares inside the polygon: %d\n", unitSquareCount);
        printf("Area of the polygon: %.2lf\n", area);

        int squareSizes[] = {16, 8, 4, 2, 1}; // Square sizes (sorted from large to small)
        int totalSquares = 0;
        for (int i = 0; i < sizeof(squareSizes) / sizeof(squareSizes[0]); i++) {
            int squareSize = squareSizes[i];
            int squareCount = area / (squareSize * squareSize);
            totalSquares += squareCount;
            area = (int)area % (squareSize * squareSize);
            printf("%dx%d squares used: %d.\n", squareSize, squareSize, squareCount);
        }
        printf("Total squares used: %d.\n", totalSquares);

        area = unitSquareCount;

        int totalPlatformCost = totalSquares * unitPlatformCost;
        int totalDrillCost = area * unitDrillCost;
        int totalCost = totalPlatformCost + totalDrillCost;
        printf("Total platform cost = %d\n", totalPlatformCost);
        printf("Total drill cost = %d\n", totalDrillCost);
        printf("Total cost = %d\n", totalCost);
        printf("Profit = %d\n", (unitSquareCount * 10) - totalCost);

        delay(100000000);
        closegraph();
        delay(100000000);
        closegraph();
    }

    return 0;
}
