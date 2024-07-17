// Author: Michela Schibuola

#include "transformation.h"
#include "detection.h"
#include "minimapConstants.h"
#include "tableOrientation.h"
#include "util.h"
#include "segmentation.h"

using namespace cv;
using namespace std;

//compute the image transformed cropped
Mat imgTransformedCropped(const Mat &img, const Mat &transform) {
    Vec<Point2f, 4> map_corners = {TOP_LEFT_MAP_CORNER, TOP_RIGHT_MAP_CORNER, BOTTOM_RIGHT_MAP_CORNER, BOTTOM_LEFT_MAP_CORNER};

    //img transformed with perspective
    Mat img_transformed;
    warpPerspective(img, img_transformed, transform,
        Size(MINIMAP_IMG_WIDTH, MINIMAP_IMG_HEIGHT));

    //img transformed cropped
    Mat img_transformed_cropped = img_transformed.rowRange(map_corners[0].y, map_corners[3].y)
                                            .colRange(map_corners[0].x, map_corners[1].x);

    return img_transformed_cropped;
}

//compute the transformation matrix using perspective transform
Mat computeTransformation(const Mat& img, const Mat& segmented, Vec<Point2f, 4>  &img_corners) {
    Vec<Point2f, 4> map_corners = {TOP_LEFT_MAP_CORNER, TOP_RIGHT_MAP_CORNER, BOTTOM_RIGHT_MAP_CORNER, BOTTOM_LEFT_MAP_CORNER};

    //compute perspective transform
    Mat transform = getPerspectiveTransform(img_corners, map_corners);

    //apply transformation considering corners such as top-left is the first one, followed by a long table side
    Mat tableSegmentedTransformed = imgTransformedCropped(segmented, transform);
    //imshow("Img transformed cropped", imgTransformed);

    //check if the transformation produces the table oriented correctly (in horizontal direction)
    if(!checkHorizontalTable(tableSegmentedTransformed)) {
        //the table is not correctly rotated

        //rotate the corners correctly
        rotateCornersClockwise(img_corners);

        //compute perspective transform with corners correctly ordered
        transform = getPerspectiveTransform(img_corners, map_corners);
        //imgTransformed = imgTransformedCropped(img, transform, map_corners);
        //imshow("Img transformed cropped", imgTransformed);
    }
    return transform;
}

Mat drawMinimap(Mat &minimap_with_track, Mat transform, vector<Ball> balls) {
    //compute balls and prec balls positions in the image
    vector<Point2f> img_balls_pos (balls.size());
    vector<Point2f> img_prec_balls_pos (balls.size());
    vector<Vec3b> ball_colors (balls.size());
    for(int i = 0; i < balls.size(); i++) {
        img_balls_pos[i] = balls[i].getBBoxCenter();
        ball_colors[i] = getColorFromCategory(balls[i].getCategory());
        img_prec_balls_pos[i] = balls[i].getBboxCenter_prec();
    }

    //compute balls and prec balls positions in the map
    vector<Point2f> map_balls_pos;
    perspectiveTransform(img_balls_pos, map_balls_pos, transform);
    vector<Point2f> map_prec_balls_pos;
    perspectiveTransform(img_prec_balls_pos, map_prec_balls_pos, transform);

    Vec<Point2f, 4> map_corners = {TOP_LEFT_MAP_CORNER, TOP_RIGHT_MAP_CORNER, BOTTOM_RIGHT_MAP_CORNER, BOTTOM_LEFT_MAP_CORNER};

    //draw tracking lines
    for(int i = 0; i < balls.size(); i++) {
        //check if a precedent ball exists, otherwise do not draw a line
        // TODO set visible to false if the ball is not visible; balls must not be copied!!!
        if(img_prec_balls_pos[i].x != -1 && img_prec_balls_pos[i].y != -1) {
            if(pointPolygonTest	(map_corners, map_balls_pos[i], false) >= 0
                && pointPolygonTest	(map_corners, map_prec_balls_pos[i], false) >= 0)
                line(minimap_with_track, map_prec_balls_pos[i], map_balls_pos[i], Vec3d(0, 0, 0), 2);
        }
    }

    //draw balls in the returned minimap
    Mat minimap_with_balls = minimap_with_track.clone();
    for(int i = 0; i < balls.size(); i++) {
        if(pointPolygonTest	(map_corners, map_balls_pos[i], false) >= 0) {
            circle(minimap_with_balls, map_balls_pos[i], MAP_BALL_RADIUS, ball_colors[i], -1);
            circle(minimap_with_balls, map_balls_pos[i], MAP_BALL_RADIUS, Vec3d(0, 0, 0), 2);
        }
    }
	return minimap_with_balls;
}
