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
static void draw_car(double*, int, int);

#define PATTERNS 8
#define BUILDINGS 6
#define CARS 2

#define CAR_1 0
#define CAR_2 1

char *vconf = "Data\\WDM_camera_flipV.xml";

int xsize, ysize;
int thresh = 100;
int count = 0;

char *cparam_name    = "Data/camera_para.dat";
ARParam cparam;

char *patt_name[PATTERNS] = { "Data/multi/marker_a.pat", "Data/multi/marker_b.pat",
                              "Data/multi/marker_c.pat", "Data/multi/marker_d.pat",
                              "Data/multi/marker_e.pat", "Data/multi/marker_g.pat",
                              "Data/multi/marker_k.pat", "Data/multi/marker_l.pat" };
int patt_id;
int hide_buildings = 0;
int hide_road = 0;

GLfloat mat_trans[PATTERNS][BUILDINGS][2] =
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

GLfloat dim[BUILDINGS][3] =
{
  { 120.0, 100.0, 315.0 },
  { 130.0, 110.0, 330.0 },
  { 100.0, 100.0, 230.0 },
  {  90.0,  90.0, 350.0 },
  { 100.0,  65.0, 250.0 },
  { 100.0, 100.0, 100.0 }
};

GLfloat vertexes[PATTERNS][4][2] =
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

GLfloat car_params[CARS][3] =
{
  {  FLT_MAX,  15.0f,  -95.0 },
  { -FLT_MAX, -30.0f, -140.0}
};

GLfloat limits[PATTERNS][2] =
{
  {    0.0, 780.0 },
  { -160.0, 620.0 },
  { -615.0, 165.0 },
  { -770.0,  10.0 },
  {    0.0, 780.0 },
  { -315.0, 465.0 },
  { -465.0, 315.0 },
  { -770.0,  10.0 },
};

GLfloat car_colors[CARS][4] =
{
  {0.5, 1.5, 2.2, 1.0},
  {2.5, 0.0, 0.5, 1.0}
};

GLfloat car_posx[CARS] =
{
   FLT_MAX,
  -FLT_MAX
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
  for (i = 0; i < PATTERNS; i++)
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
    // Sempre desenha em relação ao marcador com maior confiabilidade
    double cf = 0.0;

    for (j = 0; j < marker_num; j++)
    {
      if (marker_info[j].cf > cf)
      {
        cf = marker_info[j].cf;
        k = j;
      }
    }
    draw(&marker_info[k]);
    printf("Referencia: %s\n", patt_name[marker_info[k].id]);
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
    draw_car(gl_para, marker_info->id, CAR_1);
    draw_car(gl_para, marker_info->id, CAR_2);

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
}

static void draw_buildings(double* gl_para, int marker)
{
  GLfloat mat_ambient[BUILDINGS][4] =
  {
    {0.0, 0.0, 2.0, 1.0}, {2.0, 2.0, 0.0, 1.0}, {1.0, 0.0, 1.0, 1.0},
    {0.0, 2.0, 1.0, 1.0}, {0.0, 2.0, 2.0, 1.0}, {2.0, 1.0, 1.0, 1.0}
  };

  int building = 0;
  for (; building < BUILDINGS; building++)
  {
    glLoadMatrixd(gl_para);
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient[building]);
    glMatrixMode(GL_MODELVIEW);
    glTranslatef(mat_trans[marker][building][0], mat_trans[marker][building][1], dim[building][2] / 2);
    glScalef(dim[building][0] / 100.0, dim[building][1] / 100.0, dim[building][2] / 100.0);
    glutSolidCube(100.0);
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

static void draw_car(double* gl_para, int marker, int car)
{
  if (car_posx[car] < limits[marker][0]) car_posx[car] = limits[marker][1];
  if (car_posx[car] > limits[marker][1]) car_posx[car] = limits[marker][0];
  car_posx[car] += car_params[car][1];

  glLoadMatrixd(gl_para);
  glMaterialfv(GL_FRONT, GL_AMBIENT, car_colors[car]);
  glMatrixMode(GL_MODELVIEW);
  glTranslatef(car_posx[car], car_params[car][2] + (marker > 3 ? 240 : 0), 25.0);
  glScalef(2.0, 1.0, 1.0);
  glutSolidCube(25.0);
}
