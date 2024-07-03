// Author: Michela Schibuola

#include "transformation.h"
#include "detection.h"
#include "minimapConstants.h"
#include "tableOrientation.h"

using namespace cv;
using namespace std;

Mat imgWithTransform(Mat frame, Mat transform, Table table) {
    Vec<Point2f, 4> img_vertices = table.getBoundaries();

    //show frame perspective
    Mat frame_perspective;
    warpPerspective(frame, frame_perspective, transform,
        Size(frame.cols, frame.rows));
    //imshow("Frame perspective", frame_perspective);
    //waitKey(0);

    //show frame perspective cropped
    Vec<Point2f, 4> img_vertices_perspective;
    perspectiveTransform(img_vertices, img_vertices_perspective, transform);
    Mat frame_perspective_cropped;
    frame_perspective_cropped = frame_perspective.rowRange(img_vertices_perspective[0].y, img_vertices_perspective[3].y)
                                            .colRange(img_vertices_perspective[0].x, img_vertices_perspective[1].x);
    //imshow("Frame perspective cropped", frame_perspective_cropped);
    //waitKey(0);

    return frame_perspective_cropped;
}

//compute the transformation matrix as product of rotation and perspective
//TODO: use table or the set of edges as parameter?
Mat computeTransformation(Table &table, Mat &frame) {
    //map and img table vertices
    Vec<Point2f, 4> map_vertices = {TOP_LEFT_MAP_CORNER, TOP_RIGHT_MAP_CORNER, BOTTOM_RIGHT_MAP_CORNER, BOTTOM_LEFT_MAP_CORNER};
    Vec<Point2f, 4> img_vertices = table.getBoundaries();

    //compute perspective transform
    Mat perspectiveTransformMat = getPerspectiveTransform	(img_vertices, map_vertices);

    //apply transformation considering corners such as top-left is the first one, followed by a long table side
    Mat imgTransformed = imgWithTransform(frame, perspectiveTransformMat, table);
    imshow("First img transformed", imgTransformed);
    waitKey(0);

    //TODO: check corners_img_cropped
    //put as first point the top left which is followed by a longer edge clockwise
    Vec<Point2f, 4> corners_img_cropped =
        {   Point2f(0, 0),
            Point2f(TOP_RIGHT_MAP_CORNER.x - TOP_LEFT_MAP_CORNER.x, 0),
            Point2f(TOP_RIGHT_MAP_CORNER.x - TOP_LEFT_MAP_CORNER.x, BOTTOM_RIGHT_MAP_CORNER.y - TOP_RIGHT_MAP_CORNER.y),
            Point2f(0, BOTTOM_RIGHT_MAP_CORNER.y - TOP_RIGHT_MAP_CORNER.y)};

    //check if the transformation produces the table oriented correctly (in horizontal direction)
    if(!checkHorizontalTable(imgTransformed, corners_img_cropped)) {
        //the table is not correctly rotated
        //rotate the corners correctly
        Vec<Point2f, 4> img_vertices_temp = table.getBoundaries();
        for(int i = 0; i < 4; i++) {
            if(i+1 < 4) {
                img_vertices[i].x = img_vertices_temp[i+1].x;
                img_vertices[i].y = img_vertices_temp[i+1].y;
            }
            else {
                img_vertices[i].x = img_vertices_temp[0].x;
                img_vertices[i].y = img_vertices_temp[0].y;
            }
        }
        //update corners in table
        table.setBoundaries(img_vertices);
        //compute perspective transform with corners correctly ordered
        perspectiveTransformMat = getPerspectiveTransform(img_vertices, map_vertices);
    }
    return perspectiveTransformMat;
}

//given the ball positions and the minimap draw the balls with their category color
void drawBalls(Mat &map_img, vector<Point2f> balls_map, vector<Ball> balls) {
    //TODO: see if the position is correct (or if it starts from the top left of the image without considering the border)

    Vec3b color;
    for(int i = 0; i < balls_map.size(); i++) {
        //compute the color of the current ball using its category
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

        //TODO: draw ball with black outline
        //draw ball
        circle(map_img, balls_map[i], MAP_BALL_RADIUS, color, -1);
        circle(map_img, balls_map[i], MAP_BALL_RADIUS, Vec3d(0, 0, 0), 2);
    }
}

//TODO: vector to Vec? I don't know size, use maximum number of balls? If more than correct are detected?
//compute the positions of the balls in the minimap using transformation
vector<Point2f> computeBallsPositions(vector<Ball> &balls, const Mat &transform) {
    vector<Point2f> map_balls;
    vector<Point2f> balls_positions (balls.size());
    for(int i = 0; i < balls.size(); i++) {
        balls_positions[i] = balls[i].getBBoxCenter();
    }

    perspectiveTransform(balls_positions, map_balls, transform);

    return map_balls;
}

Mat drawMinimap(Mat &minimap, Table table, Mat frame) {
    //compute transform if not already computed
    Mat transform;
	if (!table.getTransform(transform)) {
		table.setTransform(computeTransformation(table, frame));
		table.getTransform(transform);
	}
	imgWithTransform(frame, transform, table);

    //get balls and prec balls positions
    vector<Ball> balls = *table.ballsPtr();
    vector<Point2f> balls_positions (balls.size());
    vector<Point2f> prec_balls_positions (balls.size());
    for(int i = 0; i < balls.size(); i++) {
        balls_positions[i] = balls[i].getBBoxCenter();
        prec_balls_positions[i] = balls[i].getBboxCenter_prec();
    }

    //compute balls and prec balls positions in the map
    vector<Point2f> balls_in_map;
    perspectiveTransform(balls_positions, balls_in_map, transform);
    vector<Point2f> prec_balls_in_map;
    perspectiveTransform(prec_balls_positions, prec_balls_in_map, transform);

    //draw lines for tracking (dotted)
    for(int i = 0; i < balls.size(); i++) {
        if(prec_balls_in_map[i].x >= 0 && prec_balls_in_map[i].y >= 0) {
            line(minimap, prec_balls_in_map[i], balls_in_map[i], Vec3d(0, 0, 0), 2);
            }
        }

    //draw current balls
    Mat minimap_with_balls = minimap.clone();
    //TODO: check if you want to draw someway to an outside point
    drawBalls(minimap_with_balls, balls_in_map, *(table.ballsPtr()));

    //TODO: change name of Compute ball positions to something that can be applied to other things (?)
	//vector<Point2f> ball_in_map = computeBallsPositions(*(table.ballsPtr()), transform);

    //draw corners on map with transformation
    /*
    Vec<Point2f, 4> corner_in_map;
    perspectiveTransform(table.getBoundaries(), corner_in_map, transform);
    for(int i = 0; i < 4; i++) {
        circle(minimap_with_balls, corner_in_map[i], 8, Vec3d(100, 100, 100), -1);
        if(i+1 < 4)
            line(minimap_with_balls, corner_in_map[i], corner_in_map[i+1], Vec3d(0, 0, 255), 3);
        else
            line(minimap_with_balls, corner_in_map[i], corner_in_map[0], Vec3d(0, 0, 255), 3);
    }
    */

	return minimap_with_balls;
}
