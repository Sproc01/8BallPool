// Author: Michela Schibuola

#include <opencv2/opencv.hpp>
#include "transformation.h"
#include "ObjectDetection.h"
#include "minimapConstants.h"

using namespace cv;
using namespace std;

//TODO: it is duplicated, also in tableOrientation
// Return the center point between two points
Point getCenter(Point p1, Point p2) {
    Point center;
    int dx_half = abs(p1.x - p2.x)/2;
    int dy_half = abs(p1.y - p2.y)/2;
    if(p1.x > p2.x)
        dx_half *= -1;
    if(p1.y > p2.y)
        dy_half *= -1;
    center = Point(p1.x + dx_half, p1.y + dy_half);
    return center;
}

//compute the transformation matrix as product of rotation and perspective
Mat computeTransformation(Table table) {
    Vec<Point, 4> img_vertices =  table.getBoundaries();

    //compute rotation transform
    double slope = (img_vertices[1].y - img_vertices[0].y) / (double)(img_vertices[1].x - img_vertices[0].x);
    Point center = getCenter(img_vertices[0], img_vertices[2]);
    Mat rotationTransform = getRotationMatrix2D(center, slope, 1);

    //compute perspective transform
    Vec<Point, 4> map_vertices = {TOP_LEFT_MAP_CORNER, TOP_RIGHT_MAP_CORNER, BOTTOM_RIGHT_MAP_CORNER, BOTTOM_LEFT_MAP_CORNER};
    Mat perspectiveTransform = getPerspectiveTransform	(img_vertices, map_vertices);

    return rotationTransform * perspectiveTransform;
}

//compute the positions of the balls in the minimap
vector<Point> computeBallsPositions(vector<Ball> &balls, Mat &transform) {
    vector<Point> map_balls;

    cv::transform(balls, map_balls, transform);

    return map_balls;
}

//given the ball positions and the minimap draw the balls with their category color
void drawBallsOnMap(Mat &map_img, vector<Point> balls_map, vector<Ball> balls) {

    //TODO: see if the position is correct (or if it starts from the top left of the image without considering the border
    Point position;

    //compute the color of the current ball using its category
    Vec3b color;
    for(int i = 0; i < balls_map.size(); i++) {
        position = balls_map[i];
        switch (balls[i].getCategory()) {
            case BLACK_BALL:
                color = BLACK_BGR_COLOR;
                break;
            case WHITE_BALL:
                color = WHITE_BGR_COLOR;
                break;
            case STRIPED_BALL:
                color = STRIPED_BGR_COLOR;
                break;
            case SOLID_BALL:
                color = SOLID_BGR_COLOR;
                break;
            default:
                //TODO: throw error if no correct category is found
                // throw error
                break;
        }

        circle(map_img, position, MAP_BALL_RADIUS, color, -1);
    }
}

