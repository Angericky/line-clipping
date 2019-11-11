#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <iostream>
#include <vector>
#include <cmath>
using namespace std;

struct Point2D
{
    float _x, _y;
    Point2D()
    {
        _x = 0.0f;
        _y = 0.0f;
    }
    Point2D(const Point2D& p)
    {
        _x = p._x;
        _y = p._y;
    }
    Point2D(float xx, float yy)
    {
        _x = xx;
        _y = yy;
    }
    Point2D& operator=(const Point2D& p)
    {
        _x = p._x;
        _y = p._y;
        return *this;
    }
    Point2D& operator+(const Point2D& p)
    {
        Point2D temp;
        temp._x = _x + p._x;
        temp._y = _y + p._y;
        return temp;
    }
    Point2D& operator-(const Point2D& p)
    {
        Point2D temp=Point2D(_x - p._x, _y - p._y);
        return temp;
    }
    float operator*(const Point2D& p)
    {
        return _x * p._x + _y * p._y;
    }
    
    Point2D operator*(const float k)
    {
        return Point2D(_x * k, _y * k);
    }
    
    float length()
    {
        return sqrtf(_x * _x + _y * _y);
    }
    
    void InverseDir()
    {
        _x = -_x;
        _y = -_y;
    }
};

struct Line2D
{
    Point2D _start;
    Point2D _end;
    float _length;
    
    Line2D() : _start(), _end()
    {
        _length = 0.0f;
    }
    Line2D(const Point2D& start, const Point2D& end) : _start(start), _end(end)
    {
    }
    Line2D(const Line2D& line) : _start(line._start), _end(line._end)
    {}
    
    float length()
    {
        _length = (_end - _start).length();
        return _length;
    }
    
    Line2D& operator = (const Line2D& line)
    {
        _start = line._start;
        _end = line._end;
        Line2D temp(_start, _end);
        return temp;
    }
    
    Point2D GetVector()
    {
        Point2D temp(_end._x - _start._x, _end._y - _start._y);
        return temp;
    }
};

struct Rect
{
    float _left;
    float _right;
    float _up;
    float _down;
    
    float width()
    {
        return _right - _left;
    }
    float height()
    {
        return _down - _up;
    }
};

struct Polygon
{
    int _num;//Num of lines, not points
    Point2D* points;
    Point2D* norms;
    
    Polygon()
    {
        _num = 0;
    }
    Polygon(vector<Point2D> p)
    {
        Set(p);
    }
    ~Polygon()
    {
        delete[] points;
    }
    
    Polygon(const Polygon& poly)
    {
        _num = poly._num;
        points = new Point2D[_num];
        Point2D* p = poly.points;
        for(int i = 0; i < _num; ++i)
            points[i] = p[i];
        norms = new Point2D[_num];
        for(int i = 0; i < _num; ++i)
            norms[i] = poly.norms[i];
    }
    
    Polygon& operator = (const Polygon& poly)
    {
        Polygon temp;
        temp._num = poly._num;
        temp.points = new Point2D[_num];
        Point2D* p = poly.points;
        for(int i = 0; i < _num; ++i)
            temp.points[i] = p[i];
        temp.norms = new Point2D[_num];
        for(int i = 0; i < _num; ++i)
            temp.norms[i] = poly.norms[i];
        return temp;
    }
    
    void Set(vector<Point2D> p)
    {
        _num = p.size();
        points = new Point2D[_num];
        for(int i = 0; i < _num; ++i)
            points[i] = p[i];
        
        norms = new Point2D[_num];
        ComputeNormals();
    }
    
    Line2D GetLine(int index)
    {
        Line2D temp;
        if(index >= 0 && index < _num - 1)
        {
            temp._start = points[index];
            temp._end = points[index + 1];
        }
        else if(index == _num - 1)
        {
            temp._start = points[index];
            temp._end = points[0];
        }
        return temp;
    }
    
    Point2D GetNormal(int index)
    {
        Point2D temp;
        if(index >= 0 && index < _num)
        {
            temp = norms[index];
        }
        return temp;
    }

    void ComputeNormals()
    {
        for(int i = 0; i < _num; ++i)
        {
            Line2D now = GetLine(i);
            Line2D next;
            if(i == _num - 1)
                next = GetLine(0);
            else
                next = GetLine(i + 1);
            
            Point2D v = now.GetVector();
            Point2D vn = next.GetVector();
            Point2D norm;
            if(v._x != 0)
            {
                norm._y = 1;
                norm._x = (-v._y) / v._x;
            }
            else//x and y couldn't be zero at same time
            {
                norm._x = 1;
                norm._y = (-v._x) / v._y;
            }
            
            if(norm * vn > 0)
                norm.InverseDir();
            norms[i] = norm;
        }
    }
    
    const Point2D& GetPoint(int index)
    {
        if(index >= 0 && index <= _num)
            return points[index];
        return Point2D();
    }
};

int ComputeLineCross(vector<Point2D> p)  //only draw points clockwise could judge whether it is convex or concave
{
    int _num = p.size();
    for(int i = 0; i < _num; ++i)
    {
        int pre = i == 0? _num - 1 : i - 1 ;
        int next = i == _num - 1? 0 : i + 1;
        
        Point2D A = p[pre];
        Point2D B = p[i];
        Point2D C = p[next];
        
        float cross = (B._x - A._x) * (C._y - A._y) - (B._y - A._y) * (C._x - A._x);
        if(cross < 0)
            return i;
    }
    return -1;
}

void generateConvexPolygons(vector<Point2D> points, vector<vector<Point2D>>& convexPolygons)
{
    int _num = points.size();
    int i = ComputeLineCross(points);
    if(i == -1)
        cout << "The polygen is a concave polygen." << endl;
    else
        cout << "The polygen is a convex polygen." << endl;
    
    while(i != -1){
        int pre = i - 1 < 0 ? _num - 1 : i -1;
        int next = i + 1 < _num ? i + 1 : 0;
        vector<Point2D> p{points[pre], points[i], points[next]};
        convexPolygons.push_back(p);
        points.erase(points.begin() + i);
        _num --;
        i = ComputeLineCross(points);
    }

    convexPolygons.push_back(points);

}

/*
 Global Varibles
 */
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
Point2D g_Start;
Point2D g_End;
Line2D src;
vector<Line2D> src_V;
vector<Line2D> dest_V;
bool acc;
bool buildpoly = true;
Polygon g_Poly;
vector<Polygon> convexPolygons;
int g_Count;
std::vector<Point2D> g_V;

int Cyrus_Beck(Line2D& src, Polygon& poly)
{
    float tin = 0.0f, tout = 1.0f;
    Point2D&& vec = src.GetVector();
    
    for(int i = 0; i < poly._num; ++i)
    {
        Line2D&& line = poly.GetLine(i);
        Point2D&& norm = poly.GetNormal(i);

        float nc = vec * norm;
        
        if(nc == 0)
            continue;
        else
        {
            float hit = (line._start - src._start) * norm / nc;
            if(nc > 0)//out
                tout = min(tout, hit);
            else
                tin = max(tin, hit);
        }
    }

    if(tin <= tout)
    {
        Line2D dest_T;
        dest_T._start = src._start + vec * tin;
        dest_T._end = src._start + vec * tout;
        dest_V.push_back(dest_T);
    }
    
    return tin > tout;
}

void myInit()
{
    /*
     Output Info
     */
    std::vector<Point2D> v;
    v.emplace_back();
    g_Count = 0;
    acc = false;
    
    glClearColor((float)0xee / 0x100, (float)0xee / 0x100, 1.0, 0.0);
    glColor3f(0.0f, 0.0f, 0.0f);//Map Color Black
    glPointSize(3.0);
    glMatrixMode(GL_PROJECTION);
    
    glLoadIdentity();
    gluOrtho2D(0.0, (GLdouble)SCREEN_WIDTH, (GLdouble)SCREEN_HEIGHT, 0.0);
    glViewport(0.0, SCREEN_WIDTH, 0.0, SCREEN_HEIGHT);
}

void myMouse(int button, int state, int x, int y)
{
    if(buildpoly)
    {
        if(button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
        {
            vector<vector<Point2D>> convexPolygonsVectors;
            generateConvexPolygons(g_V, convexPolygonsVectors);
            cout << "vector size: "<< convexPolygonsVectors.size() << endl;
            for(int i = 0; i < convexPolygonsVectors.size(); i ++){
                Polygon poly(convexPolygonsVectors[i]);
                convexPolygons.push_back(poly);
            }
            //build over
            g_Poly.Set(g_V);
            g_V.clear();

            buildpoly = false;
            cout << "Build Poly Over.";
            glutPostRedisplay();
        }
        if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
        {
            g_V.emplace_back(x, y);
            cout << "Add Point: (" << x << ", " << y << ").\n";
            glutPostRedisplay();
        }
        return;
    }
    
    if(button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
    {
        buildpoly = true;
        g_Count = 0;
        glutPostRedisplay();
        return;
    }
    
    if(button != GLUT_LEFT_BUTTON || state != GLUT_DOWN)
        return;
    
    cout << "MyMouse Called with " << x << ", " << y << endl;
    switch(g_Count)
    {
        case 0:
        {
            ++g_Count;
            g_Start._x = x;
            g_Start._y = y;
            src._start = g_Start;
        }break;
        case 1:
        {
            ++g_Count;
            g_End._x = x;
            g_End._y = y;
            src._end = g_End;
            printf("(%.0f, %.0f), (%.0f, %.0f)\n",src._start._x,src._start._y, src._end._x, src._end._y);
            dest_V.clear();
            for(int i = 0; i < convexPolygons.size(); i ++){
                acc = !Cyrus_Beck(src, convexPolygons[i]);
                if(!acc)
                {
                    cout << "Refused.\n";
                    break;
                }
                else
                    cout << "Accept.\n";
            }
            
            glutPostRedisplay();
        }break;
        case 2:
        {
            g_Start._x = x;
            g_Start._y = y;
            src._start = g_Start;
            g_Count = 1;
        }break;
    }
}

void myDisplay()
{
    glClear(GL_COLOR_BUFFER_BIT);
    int width = 2.0f;
    Point2D temp;
    if(buildpoly)
    {
        glColor3f((float)0 / 255, (float)64 / 255, (float)152 / 255);//Poly
        glPointSize(3.0);
        glLineWidth(width);
        glBegin(GL_LINE_STRIP);
        
        for(int i = 0; i < g_V.size(); ++i)
            glVertex2d(g_V[i]._x, g_V[i]._y);
        
        glEnd();
    }
    else
    {
        glColor3f((float)0 / 255, (float)64 / 255, (float)152 / 255);//Poly
        glPointSize(3.0);
        glLineWidth(width);
        glBegin(GL_LINE_STRIP);
        
        for(int i = 0; i < g_Poly._num; ++i)
        {
            temp = g_Poly.GetPoint(i);
            glVertex2d(temp._x, temp._y);
        }
        temp = g_Poly.GetPoint(0);
        glVertex2d(temp._x, temp._y);
        
        glEnd();
        
        if(g_Count == 2)
        {
            //Draw Line
            glColor3f((float)249 / 255, (float)161 / 255, (float)41 / 255);//Normal Line, Orange
            glPointSize(2.0);
            glBegin(GL_LINES);
            
            vector<Point2D> p;
            p.push_back(dest_V[dest_V.size() - 1]._start);
            
            for(int i = 0; i < dest_V.size() - 1 ; ++i){
                p.push_back(dest_V[i]._start);
                p.push_back(dest_V[i]._end);
            }
            
            p.push_back(dest_V[dest_V.size() - 1]._end);

            dest_V.clear();
            for(int i = 0; i < p.size() - 1; i+=2){
                Line2D temp(p[i], p[i + 1]);
                dest_V.push_back(temp);
            }
            cout << "num of line segments after clipping: " << dest_V.size() << endl;

            glVertex2d(src._start._x, src._start._y);
            glVertex2d(src._end._x, src._end._y);
            
            cout << "\nDraw Line\n";
            if(acc)
            {
                //Draw Cutted Line
                glColor3f((float)217 / 255, (float)227 / 255, (float)103 / 255);//Normal Line, Green
                glPointSize(3.0);
                for(int i = 0; i < dest_V.size(); ++i){
                    Line2D dest = dest_V[i];
                    glVertex2d(dest._start._x, dest._start._y);
                    glVertex2d(dest._end._x, dest._end._y);
                    cout << "dest: (" << dest._start._x << ", " << dest._start._y << ')'
                        << ", (" << dest._end._x << ", " << dest._end._y << ')' << endl;
                }
                cout << "\nDraw CutLine\n";
            }
            glEnd();
        }
    }
    
    //glutSwapBuffers();
    glFlush();
    //cout << "Render Over\n";
}

int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    //glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Cyrus_Beck");
    glutDisplayFunc(myDisplay);
    glutMouseFunc(myMouse);
    
    myInit();
    glutMainLoop();
    
    return 0;
}


