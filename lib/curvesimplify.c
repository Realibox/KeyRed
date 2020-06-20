//
// Created by Yang Lee on 2020/5/25.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "curvesimplify.h"

struct KeyFrameList {
    struct KeyFrameNode *first;

    struct KeyFrameNode *start;
    struct KeyFrameNode *end;
};

struct KeyFrameList frameList = {NULL, NULL, NULL};

void add_node_to_list(KeyFrame *key) {
    struct KeyFrameNode *newKey = malloc(sizeof(struct KeyFrameNode));
    if (newKey == NULL) {
        printf("Error: malloc failed in add_to list\n");
        exit(EXIT_FAILURE);
    }

    frameList.start->next = newKey;

    newKey->key = key;
    newKey->prev = frameList.start;
    newKey->next = frameList.end;

    frameList.end->prev = newKey;
    frameList.end = newKey;
}

double get_tangent(KeyFrame key1, KeyFrame key2) {
    return (key1.value - key2.value) / (key1.time - key2.time);
}

double get_point_to_line_dis(const KeyFrame *p, const KeyFrame *p1, const KeyFrame *p2) {
    double A, B, C, distant;
    A = (p1->value - p2->value) / (p1->time - p2->time);
    B = -1;
    C = p1->value - A * p1->time;
    distant = fabs(A * p->time + B * p->value + C) / sqrt(pow(A, 2) + pow(B, 2));

    return distant;
}

KeyFrame *get_max_error_index(KeyFrame *curve) {
    KeyFrame *startPoint = frameList.start->key;
    KeyFrame *endPoint = frameList.end->key;

    double max_error = -1;
    int max_error_index = -1;

    for (int i = startPoint->index + 1; i < endPoint->index; ++i) {
        double temp;
        temp = get_point_to_line_dis(curve + i, startPoint, endPoint);
        if (temp - max_error > 0.0001) {
            max_error = temp;
            max_error_index = i;
        }
    }

    curve[max_error_index].leftTangent = get_tangent(*startPoint, *endPoint);
    curve[max_error_index].rightTangent = curve[max_error_index].leftTangent;

    return &curve[max_error_index];
}

double evaluate_internal(double curTime, const KeyFrame *leftKey, const KeyFrame *rightKey) {
    double leftTime = leftKey->time;
    double leftValue = leftKey->value;
    double leftTan = leftKey->rightTangent;

    double rightTime = rightKey->time;
    double rightValue = rightKey->value;
    double rightTan = rightKey->leftTangent;

    double t = (curTime - leftTime) / (rightTime - leftTime);
    double scale = rightTime - leftTime;

    double h00 = 2 * pow(t, 3) - 3 * pow(t, 2) + 1;
    double h10 = 1 * pow(t, 3) - 2 * pow(t, 2) + t;
    double h01 = -2 * pow(t, 3) + 3 * pow(t, 2);
    double h11 = 1 * pow(t, 3) - 1 * pow(t, 2);

    double v = h00 * leftValue
               + h10 * scale * leftTan
               + h01 * rightValue
               + h11 * scale * rightTan;
    return v;
}

bool evaluate_error(const KeyFrame *curve, double error) {
    KeyFrame *s = frameList.start->key;
    KeyFrame *e = frameList.end->key;

    for (int i = s->index + 1; i < e->index; ++i) {
        double v = evaluate_internal(curve[i].time, s, e);
        double cur_error = fabs(v - curve[i].value);
        if (cur_error > error) {
            return false;
        }
    }

    return true;
}

struct KeyFrameNode *simplify_curve(Point *points, int n, double error) {
    KeyFrame *curve = malloc(n * sizeof(KeyFrame));

    for (int i = 0; i < n; ++i) {
        curve[i].index = i;
        curve[i].time = points[i].t;
        curve[i].value = points[i].v;
        curve[i].leftTangent = 0;
        curve[i].rightTangent = 0;
    }

    curve[0].rightTangent = get_tangent(curve[0], curve[1]);
    struct KeyFrameNode *firstNode = malloc(sizeof(struct KeyFrameNode));
    firstNode->key = &curve[0];
    firstNode->prev = NULL;
    firstNode->next = NULL;
    frameList.start = frameList.first = firstNode;

    curve[n - 1].leftTangent = get_tangent(curve[n - 2], curve[n - 1]);
    struct KeyFrameNode *finalNode = malloc(sizeof(struct KeyFrameNode));
    finalNode->key = &curve[n - 1];
    finalNode->prev = NULL;
    finalNode->next = NULL;
    frameList.end = finalNode;

    frameList.start->next = frameList.end;
    frameList.end->prev = frameList.start;

    while (true) {
        bool accord_error = false;
        accord_error = evaluate_error(&curve[0], error);
        if (!accord_error) {
            while (!accord_error) {
                KeyFrame *errorKey = get_max_error_index(&curve[0]);
                add_node_to_list(errorKey);
                accord_error = evaluate_error(&curve[0], error);
            }
        }

        frameList.start = frameList.start->next;

        if (frameList.end->next == NULL ) {
            accord_error = evaluate_error(&curve[0], error);
            if (accord_error) {
                break;
            }
        }

        frameList.end = frameList.end->next;

        if (frameList.end->next == NULL ) {
            accord_error = evaluate_error(&curve[0], error);
            if (accord_error) {
                break;
            }
        }
    }

    return frameList.first;
}