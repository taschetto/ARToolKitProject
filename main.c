#include <windows.h>
#include <VideoIM.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <AR/gsub.h>
#include <AR/video.h>
#include <AR/param.h>
#include <AR/ar.h>
#include <float.h>

static void init(void);
static void cleanup(void);
static void keyEvent(unsigned char, int, int);
static void mainLoop(void);
static void draw(ARMarkerInfo*);
static void draw_buildings(double*, int);
static void draw_road(double*, int);
static void draw_car1(double*, int);
static void draw_car2(double*, int);

#define PATTERN_COUNT 8
#define BUILDING_COUNT 6
#define CAR_COUNT 2

char *vconf = "Data\\WDM_camera_flipV.xml";

int xsize, ysize;
int thresh = 100;
int count = 0;

char *cparam_name    = "Data/camera_para.dat";
ARParam cparam;

char *patt_name[PATTERN_COUNT] = { "Data/multi/marker_a.pat", "Data/multi/marker_b.pat",
                                   "Data/multi/marker_c.pat", "Data/multi/marker_d.pat",
                                   "Data/multi/marker_e.pat", "Data/multi/marker_g.pat",
                                   "Data/multi/marker_k.pat", "Data/multi/marker_l.pat" };
int patt_id;
int hide_buildings = 0;
int hide_road = 0;

GLfloat mat_trans[PATTERN_COUNT][BUILDING_COUNT][2] =
{
  {{  315.0,    0.0 }, {  465.0,    0.0 }, { 850.0, -110.0 }, {  615.0, -230.0 }, {  170.0, -230.0 }, {  -70.0, -110.0 }},
  {{  155.0,    0.0 }, {  305.0,    0.0 }, { 690.0, -110.0 }, {  455.0, -230.0 }, {   10.0, -230.0 }, { -230.0, -110.0 }},
  {{ -300.0,    0.0 }, { -150.0,    0.0 }, { 235.0, -110.0 }, {    0.0, -230.0 }, { -445.0, -230.0 }, { -685.0, -110.0 }},
  {{ -455.0,    0.0 }, { -305.0,    0.0 }, {  80.0, -110.0 }, { -155.0, -230.0 }, { -610.0, -230.0 }, { -840.0, -110.0 }},
  {{  315.0,  240.0 }, {  465.0,  240.0 }, { 850.0,  130.0 }, {  615.0,   10.0 }, {  170.0,   10.0 }, {  -70.0,  130.0 }},
  {{    0.0,  240.0 }, {  150.0,  240.0 }, { 535.0,  130.0 }, {  300.0,   10.0 }, { -145.0,   10.0 }, { -385.0,  130.0 }},
  {{ -150.0,  240.0 }, {    0.0,  240.0 }, { 385.0,  130.0 }, {  150.0,   10.0 }, { -295.0,   10.0 }, { -535.0,  130.0 }},
  {{ -455.0,  240.0 }, { -305.0,  240.0 }, {  80.0,  130.0 }, { -155.0,   10.0 }, { -610.0,   10.0 }, { -840.0,  130.0 }}
};

GLfloat vertexes[PATTERN_COUNT][4][2] =
{
  {{    0.0, -60.0 }, {    0.0, -180.0 }, { 780.0, -180.0 }, { 780.0, -60.0 }},
  {{ -160.0, -60.0 }, { -160.0, -180.0 }, { 620.0, -180.0 }, { 620.0, -60.0 }},
  {{ -615.0, -60.0 }, { -615.0, -180.0 }, { 165.0, -180.0 }, { 165.0, -60.0 }},
  {{ -770.0, -60.0 }, { -770.0, -180.0 }, {  10.0, -180.0 }, {  10.0, -60.0 }},
  {{    0.0, 180.0 }, {    0.0,   60.0 }, { 780.0,   60.0 }, { 780.0, 180.0 }},
  {{ -315.0, 180.0 }, { -315.0,   60.0 }, { 465.0,   60.0 }, { 465.0, 180.0 }},
  {{ -465.0, 180.0 }, { -465.0,   60.0 }, { 315.0,   60.0 }, { 315.0, 180.0 }},
  {{ -770.0, 180.0 }, { -770.0,   60.0 }, {  10.0,   60.0 }, {  10.0, 180.0 }}
};

  GLfloat limits[CAR_COUNT][8][2] =
  {
    {{   0.0, 780.0 }, { -160.0, 620.0 }, { -615.0,  165.0 }, { -770.0,   10.0 }, {   0.0, 780.0 }, { -315.0,  465.0 }, { -465.0,  315.0 }, { -770.0,   10.0 }},
    {{ 780.0,   0.0 }, {  620.0, -160.0 }, { 165.0, -615.0 }, {   10.0, -770.0 }, { 780.0,   0.0 }, {  465.0, -315.0 }, {  315.0, -465.0 }, {   10.0, -770.0 }}
  };

int main(int argc, char **argv)
{
  printf ("Diretorio corrente:");
  system("cd");
  printf("\n");
  glutInit(&argc, argv);
  init();
  arVideoCapStart();
  argMainLoop( NULL, keyEvent, mainLoop );
  return (0);
}

static void keyEvent( unsigned char key, int x, int y)
{
  if (key == 13) hide_road = !hide_road;
  if (key == 32) hide_buildings = !hide_buildings;

  if (key == 0x1b)
  {
    printf("*** %f (frame/sec)\n", (double)count/arUtilTimer());
    cleanup();
    exit(0);
  }
}

static void init( void )
{
  ARParam  wparam;
  if( arVideoOpen( vconf ) < 0 ) exit(0);
  if( arVideoInqSize(&xsize, &ysize) < 0 ) exit(0);
  printf("Image size (x,y) = (%d,%d)\n", xsize, ysize);

  if( arParamLoad(cparam_name, 1, &wparam) < 0 )
  {
    printf("Camera parameter load error !!\n");
    getchar();
    exit(0);
  }
  arParamChangeSize( &wparam, xsize, ysize, &cparam );
  arInitCparam( &cparam );
  printf("*** Camera Parameter ***\n");
  arParamDisp( &cparam );

  int i = 0;
  for (i = 0; i < PATTERN_COUNT; i++)
  {
    if( (patt_id=arLoadPatt(patt_name[i])) < 0 )
    {
      printf("pattern load error !!\n");
      getchar();
      exit(0);
    }
    printf("Loaded patt_id=%d\n", patt_id);
  }

  argInit( &cparam, 1.0, 0, 0, 0, 0 );
}

static void cleanup(void)
{
  arVideoCapStop();
  arVideoClose();
  argCleanup();
}

static void mainLoop(void)
{
  ARUint8 *dataPtr;
  ARMarkerInfo *marker_info;
  int marker_num, j, k;

  if ((dataPtr = (ARUint8 *)arVideoGetImage()) == NULL)
  {
    arUtilSleep(2);
    return;
  }

  if (count++ == 0) arUtilTimerReset();
  argDrawMode2D();
  argDispImage(dataPtr, 0,0);
  if(arDetectMarker(dataPtr, thresh, &marker_info, &marker_num) < 0)
  {
    cleanup();
    exit(0);
  }

  arVideoCapNext();

  if (marker_num > 0)
  {
    // Sempre desenha em relação ao primeiro marcador que encontrar
    draw(&marker_info[0]);
    printf("Referencia: %s\n", patt_name[marker_info[0].id]);
  }

  argSwapBuffers();
}

static void draw(ARMarkerInfo* marker_info)
{
    GLfloat mat_flash[]       = {0.0, 0.0, 1.0, 1.0};
    GLfloat mat_flash_shiny[] = {50.0};
    GLfloat light_position[]  = {100.0,-200.0,200.0,0.0};
    GLfloat ambi[]            = {0.1, 0.1, 0.1, 0.1};
    GLfloat lightZeroColor[]  = {0.9, 0.9, 0.9, 0.1};

    double gl_para[16];
    double patt_width = 80.0;
    double patt_center[2] = {0.0, 0.0};
    double patt_trans[3][4];

    argDrawMode3D();
    argDraw3dCamera(0, 0);
    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    arGetTransMat(marker_info, patt_center, patt_width, patt_trans);

    argConvGlpara(patt_trans, gl_para);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd( gl_para );

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambi);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_flash);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_flash_shiny);

    if (hide_buildings) glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    draw_buildings(gl_para, marker_info->id);
    if (hide_buildings) glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    if (!hide_road) draw_road(gl_para, marker_info->id);
    draw_car1(gl_para, marker_info->id);
    draw_car2(gl_para, marker_info->id);

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
}

static void draw_buildings(double* gl_para, int marker)
{
  GLfloat mat_ambient[BUILDING_COUNT][4] =
  {
    {0.0, 0.0, 2.0, 1.0}, {2.0, 2.0, 0.0, 1.0}, {1.0, 0.0, 1.0, 1.0},
    {0.0, 2.0, 1.0, 1.0}, {0.0, 2.0, 2.0, 1.0}, {2.0, 1.0, 1.0, 1.0}
  };

  GLfloat height[] = { 100.0, 100.0, 100.0,
                       100.0, 100.0, 100.0 };

  int building = 0;
  for (; building < BUILDING_COUNT; building++)
  {
    glLoadMatrixd(gl_para);
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient[building]);
    glMatrixMode(GL_MODELVIEW);
    glTranslatef(mat_trans[marker][building][0], mat_trans[marker][building][1], height[building]);
    glScalef(1.0, 1.0, 2.0);
    glutSolidCube(height[building]);
  }
}

static void draw_road( double* gl_para, int marker )
{
  GLfloat mat_ambient[] = {0.6, 0.6, 0.6, 2.0};

  glLoadMatrixd( gl_para );
  glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
  glMatrixMode(GL_MODELVIEW);

  glBegin(GL_QUADS);
  int vertex = 0;
  for (; vertex < 4; vertex++) glVertex3f(vertexes[marker][vertex][0], vertexes[marker][vertex][1], 0.0f);
  glEnd();
}

static void draw_car1(double* gl_para, int marker)
{
  GLfloat mat_ambient[] = {0.5, 1.5, 2.2, 1.0};
  static float x_pos = FLT_MAX;

  if (x_pos > limits[0][marker][1]) x_pos = limits[0][marker][0];
  x_pos += 15.0f;

  glLoadMatrixd(gl_para);
  glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
  glMatrixMode(GL_MODELVIEW);
  glTranslatef(x_pos , -95.0 + (marker > 3 ? 240 : 0), 25.0);
  glScalef(2.0, 1.0, 1.0);
  glutSolidCube(25.0);
}

static void draw_car2( double* gl_para, int marker )
{
  GLfloat mat_ambient[] = {2.5, 0.0, 0.5, 1.0};
  static float x_pos = -FLT_MAX;

  if (x_pos < limits[1][marker][1]) x_pos = limits[1][marker][0];
  x_pos -= 30.0f;

  glLoadMatrixd(gl_para);
  glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
  glMatrixMode(GL_MODELVIEW);
  glTranslatef(x_pos , -140.0 + (marker > 3 ? 240 : 0), 25.0);
  glScalef(2.0, 1.0, 1.0);
  glutSolidCube(25.0);
}
