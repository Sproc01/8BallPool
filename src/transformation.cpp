// Author: Michela Schibuola

#include "transformation.h"
#include "detection.h"
#include "minimapConstants.h"
#include "tableOrientation.h"

using namespace cv;
using namespace std;

//TODO: it is duplicated, also in tableOrientation
// Return the center point between two points
Point getCenter2(Point p1, Point p2) {
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

Mat imgWithTransform(Mat frame, Mat transform, Table table) {
    //TODO: remove this, should be already in table (float points)
    vector<Point2f> img_vertices (4);
    Vec<Point, 4> img_vertices_temp = table.getBoundaries();
    for(int i = 0; i < 4; i++) {
        img_vertices[i].x = (float)img_vertices_temp[i].x;
        img_vertices[i].y = (float)img_vertices_temp[i].y;
    }

    //show frame perspective
    Mat frame_perspective;
    warpPerspective(frame, frame_perspective, transform,
        Size(frame.cols, frame.rows));
    imshow("Frame perspective", frame_perspective);
    //waitKey(0);

    //show frame perspective cropped
    vector<Point2f> img_vertices_perspective (4);
    perspectiveTransform(img_vertices, img_vertices_perspective, transform);
    Mat frame_perspective_cropped;
    frame_perspective_cropped = frame_perspective.rowRange(img_vertices_perspective[0].y, img_vertices_perspective[3].y)
                                            .colRange(img_vertices_perspective[0].x, img_vertices_perspective[1].x);
    imshow("Frame perspective cropped", frame_perspective_cropped);
    //waitKey(0);

    return frame_perspective_cropped;
}

//compute the transformation matrix as product of rotation and perspective
//TODO: use table or the set of edges as parameter?
Mat computeTransformation(Table table, Mat &frame) {
    //TODO: set all Point to Point2f??
    //convert vertices vector to vector of Point2f (needed for getPerspectiveTransform)
    vector<Point2f> map_vertices = {TOP_LEFT_MAP_CORNER, TOP_RIGHT_MAP_CORNER, BOTTOM_RIGHT_MAP_CORNER, BOTTOM_LEFT_MAP_CORNER};
    vector<Point2f> img_vertices (4);
    Vec<Point, 4> img_vertices_temp = table.getBoundaries();
    for(int i = 0; i < 4; i++) {
        img_vertices[i].x = (float)img_vertices_temp[i].x;
        img_vertices[i].y = (float)img_vertices_temp[i].y;
    }

    //compute perspective transform
    Mat perspectiveTransformMat = getPerspectiveTransform	(img_vertices, map_vertices);

    //TODO: verify if the image il correctly transformed
    //put as first point the top left which is followed by a longer edge clockwise
    Vec<Point2f, 4> corners_img_cropped =
        {   Point2f(0, 0),
            Point2f(TOP_RIGHT_MAP_CORNER.x - TOP_LEFT_MAP_CORNER.x, 0),
            Point2f(TOP_RIGHT_MAP_CORNER.x - TOP_LEFT_MAP_CORNER.x, BOTTOM_RIGHT_MAP_CORNER.y - TOP_RIGHT_MAP_CORNER.y),
            Point2f(0, BOTTOM_RIGHT_MAP_CORNER.y - TOP_RIGHT_MAP_CORNER.y)};
    if(!checkHorizontalTable(imgWithTransform(frame, perspectiveTransformMat, table), corners_img_cropped)) {
        //compute transform with the corners rotated
        //TODO: remove this with Point2f
        Vec<Point, 4> img_vertices_temp = table.getBoundaries();
        for(int i = 0; i < 4; i++) {
            if(i+1 < 4) {
                img_vertices[i].x = (float)img_vertices_temp[i+1].x;
                img_vertices[i].y = (float)img_vertices_temp[i+1].y;
            }
            else {
                img_vertices[i].x = (float)img_vertices_temp[0].x;
                img_vertices[i].y = (float)img_vertices_temp[0].y;
            }
        }
        for(int i = 0; i < 4; i++) {
                img_vertices_temp[i].x =  static_cast<int>(img_vertices[i].x);
                img_vertices_temp[i].y = static_cast<int>(img_vertices[i].y);
        }
        table.setBoundaries(img_vertices_temp);
        //compute perspective transform correct
        perspectiveTransformMat = getPerspectiveTransform	(img_vertices, map_vertices);
    }
    return perspectiveTransformMat;

}

//compute the positions of the balls in the minimap
vector<Point2f> computeBallsPositions(vector<Ball> &balls, Mat &transform) {
    vector<Point2f> map_balls;
    vector<Point2f> balls_positions (balls.size());
    for(int i = 0; i < balls.size(); i++) {
        balls_positions[i] = balls[i].getBBoxCenter();
    }

    perspectiveTransform(balls_positions, map_balls, transform);

    return map_balls;
}

//given the ball positions and the minimap draw the balls with their category color
void drawBallsOnMap(Mat &map_img, vector<Point2f> balls_map, vector<Ball> balls) {

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

Mat minimapWithBalls(Mat minimap, Table table, Mat frame) {
    table.setTransform(computeTransformation(table, frame));
    //TODO: fix table which changes order of points if the orientation is not correct
    imgWithTransform(frame, table.getTransform(), table);
    vector<Point2f> ball_in_map = computeBallsPositions(*(table.getBalls()), table.getTransform());
    drawBallsOnMap(minimap, ball_in_map, *(table.getBalls()));

    return minimap;
}


