/*!\file window.c
 * \brief Utilisation du raster "maison" pour visualisation en vue du
 * dessus d'un début de casse briques.
 * \author Farès BELHADJ, amsi@up8.edu
 * \date December 4, 2020.
 *
 * A titre indicatif, temps de programmation de cet exemple : 58
 * minutes, en partant de sc_00_07_moteur-0.3.tgz, commentaires
 * compris.
 */
#include <assert.h>
#include <time.h>
/* inclusion des entêtes de fonctions de gestion de primitives simples
 * de dessin. La lettre p signifie aussi bien primitive que
 * pédagogique. */
#include <GL4D/gl4dp.h>
/* inclure notre bibliothèque "maison" de rendu */
#include "moteur.h"

/* inclusion des entêtes de fonctions de création et de gestion de
 * fenêtres système ouvrant un contexte favorable à GL4dummies. Cette
 * partie est dépendante de la bibliothèque SDL2 */
#include <GL4D/gl4duw_SDL2.h>
/* dans plateau.c, vous avez le droit de changer de générateur ... */
extern unsigned int *plateau(int w, int h);
/* protos de fonctions locales (static) */
static void init(void);
static void simu(void);
static void draw(void);
static void sortie(void);
static void print_objects(void);
static void keydown_func(int);
static void init_bricks();
static void collision_tests(void);
/*!\brief une surface représentant un quadrilatère */
static surface_t *_quad = NULL;
/*!\brief une surface représentant un cube */
static surface_t *_cube = NULL;
/*!\brief une surface représentant un Pad */
static surface_t *_paddle = NULL;
/*!\brief une surface représentant une sphère (pour la balle) */
static surface_t *_sphere = NULL;
static liste_t *_bricks = NULL;
typedef struct brick_t
{
  surface_t *brick_surface;
  vec3 brick_coords;
  vec3 brick_size;
} brick_t;

/*!\brief le plateau */
static unsigned int *_plat = NULL;
/*!\brief la largeur du plateau */
static const int _pW = 11;
/*!\brief la hauteur du plateau */
static const int _pH = 21;
/*!\brief la largeur de la fenêtre */
static const int _wW = 320;
/*!\brief la hauteur de la fenêtre */
static const int _wH = 512;
//!  translate Z du paddle paddle
/*!\brief ma balle est codée sur un vec3, x est x, y est z et z est
 * son rayon */
static vec3 _ball = {0, 0, 0.84f};
static vec4 _paddle_data = {0, 1.0f, -_pH + 3, 3.1f};
float _mouv_coord = 0.80f;

/*!\brief ma balle a une vitesse, x est vx, y est vz */
static vec2 _ballv = {0, 0};
/*!\brief une accélération, exemple la gravité */
static vec2 _g = {0, -9.8f * 10.0f /* facteur réel/virtuel, dépend de la taille de l'écran */};

/*!\brief paramètre l'application et lance la boucle infinie. */
int main(int argc, char **argv)
{
  /* tentative de création d'une fenêtre pour GL4Dummies */
  if (!gl4duwCreateWindow(argc, argv,                       /* args du programme */
                          "Mon moteur de rendu <<Maison>>", /* titre */
                          10, 10, _wW, _wH,                 /* x, y, largeur, heuteur */
                          GL4DW_SHOWN) /* état visible */)
  {
    /* ici si échec de la création souvent lié à un problème d'absence
     * de contexte graphique ou d'impossibilité d'ouverture d'un
     * contexte OpenGL (au moins 3.2) */
    return 1;
  }
  /* Pour forcer la désactivation de la synchronisation verticale */
  SDL_GL_SetSwapInterval(0);
  init();
  /* création d'un screen GL4Dummies (texture dans laquelle nous
   * pouvons dessiner) aux dimensions de la fenêtre */
  gl4dpInitScreen();
  /* mettre en place la fonction simu */
  gl4duwKeyDownFunc(keydown_func);
  gl4duwIdleFunc(simu);

  /* mettre en place la fonction de display */
  gl4duwDisplayFunc(draw);
  /* boucle infinie pour éviter que le programme ne s'arrête et ferme
   * la fenêtre immédiatement */
  gl4duwMainLoop();
  return 0;
}

/*!\brief init de nos données, spécialement les trois surfaces
 * utilisées dans ce code */
void init(void)
{
  _bricks = list_init(sizeof(brick_t *));
  parsing();
  GLuint id;
  //ANCHOR couleurs des elements
  vec4 r = {1.0f, 0.0f, 0.0f, 1}, g = {0.0f, 1.0f, 0.0f, 1}, b = {1.0f, 1.0f, 0.7f, 1};
  /* on créé nos 3 types de surfaces */
  //ANCHOR Interieur du casse brique
  _quad = mkQuad(); /* ça fait 2 triangles        */
  // ANCHOR Bordure du casse brique
  _cube = mkCube(); /* ça fait 2x6 triangles      */
  // ANCHOR Le pad de jeu
  _paddle = mkCube(); /* ça fait 2x6 triangles      */
  // ANCHOR balle
  _sphere = mkSphere(12, 4); /* ça fait 2x12x5 triangles   */
  for (cell_t *encours = _bricks->premier ; encours != NULL ; encours++){
    brick_t* une_brique = encours->data;
    une_brique -> brick_surface = mkCube();
    une_brique -> brick_surface->dcolor = r;
  }

  /* on change les couleurs de surfaces */
  _sphere->dcolor = r;
  _quad->dcolor = g;
  _cube->dcolor = b;
  _paddle->dcolor = b;
  /* on leur rajoute la texture (cube et quad) */
  setTexId(_quad, id = getTexFromBMP("images/tex.bmp"));
  setTexId(_cube, id);
  setTexId(_paddle, id);
  /* on active l'utilisation de la texture pour les deux */
  enableSurfaceOption(_quad, SO_USE_TEXTURE);
  enableSurfaceOption(_cube, SO_USE_TEXTURE);
  enableSurfaceOption(_paddle, SO_USE_TEXTURE);
  /* on active l'ombrage */
  enableSurfaceOption(_quad, SO_USE_LIGHTING);
  enableSurfaceOption(_cube, SO_USE_LIGHTING);
  enableSurfaceOption(_paddle, SO_USE_LIGHTING);
  enableSurfaceOption(_sphere, SO_USE_LIGHTING);
  _plat = plateau(_pW, _pH);
  /* initialiser aléatoirement la vitesse de la balle (au pifomètre) */
  srand(time(NULL));
  _ballv.x = ((2.0 * (rand() / (RAND_MAX + 1.0))) - 1.0) * _pW * 2.0;
  _ballv.y = 1.5 * _pH + (rand() / (RAND_MAX + 1.0)) * _pH;
  /* mettre en place la fonction à appeler en cas de sortie */
  atexit(sortie);
}

/*!\brief à appeler à chaque simulation (avant draw). */
void simu(void)
{
  static int firstTime = 1;
  static double t0 = 0;
  double t, dt;
  t = gl4dGetElapsedTime();
  dt = (t - t0) / 1000.0;
  t0 = t;
  if (firstTime)
  {
    firstTime = 0;
    return;
  }
  collision_tests();

  /* physique de base : mécanique newtonienne */
  _ball.x += _ballv.x * dt;
  _ball.y += _ballv.y * dt;
}

void collision_tests()
{

  if (_paddle_data.x - _paddle_data.w <= -_pW + 2.0f)
  {
    _paddle_data.x += _mouv_coord;
  }
  if (_paddle_data.x + _paddle_data.w >= _pW - 2.0f)
  {
    _paddle_data.x -= _mouv_coord;
  }

  if (_ball.x >= _paddle_data.x - _paddle_data.w && _ball.x <= _paddle_data.x + _paddle_data.w)
  {
    if (_ball.y + _pH - 4.0f <= _paddle_data.y)
    {
      _ballv.x = -_ballv.x;
      _ballv.y = _ballv.y;
    }
  }

  float lg = -_pW + 2.0f + _ball.z; /* limite gauche */
  float ld = _pW - 2.0f - _ball.z;  /* limite droite */
  float lb = _pH - 2.0f - _ball.z;  /* limite basse */
  float lh = -_pH + 2.0f + _ball.z; /* limite haute */
  if (_ball.x <= lg || _ball.x >= ld)
    _ballv.x = -_ballv.x;
  if (_ball.y <= lh || _ball.y >= lb)
  {
    _ballv.y = -_ballv.y;
  }
}

static void keydown_func(int keypressed)
{

  switch (keypressed)
  {
  case GL4DK_LEFT:
    _paddle_data.x += _mouv_coord;
    break;
  case GL4DK_RIGHT:
    _paddle_data.x -= _mouv_coord;
    break;
  case GL4DK_q:
    _paddle_data.x += _mouv_coord;
    break;
  case GL4DK_d:
    _paddle_data.x -= _mouv_coord;

  default:
    break;
  }
}

/*!\brief la fonction appelée à chaque display. */
void draw(void)
{
  int i, j;
  float mvMat[16], projMat[16], nmv[16];
  /* effacer l'écran et le buffer de profondeur */
  gl4dpClearScreen();
  clearDepth();
  /* des macros facilitant le travail avec des matrices et des
   * vecteurs se trouvent dans la bibliothèque GL4Dummies, dans le
   * fichier gl4dm.h */
  /* charger un frustum dans projMat */
  MFRUSTUM(projMat, -0.05f, 0.05f,
           -0.05f * _wH / _wW /* pour garder le ratio */,
           0.05f * _wH / _wW /* pour garder le ratio */,
           0.1f, 1000.0f);
  /* charger la matrice identité dans model-view */
  MIDENTITY(mvMat);
  /* on place la caméra en vue du dessus */
  lookAt(mvMat, 0, 30, -1, 0, 0, 0, 0, 1, 0);
  /* le quadrilatère est agrandi et couché */
  memcpy(nmv, mvMat, sizeof nmv); /* copie mvMat dans nmv */
  rotate(nmv, -90.0f, 1.0f, 0.0f, 0.0f);
  scale(nmv, _pW - 1, _pH - 1, 1.0f);
  transform_n_raster(_quad, nmv, projMat);
  /* le cube est relevé (+1 en y) et placé là où il y a des murs */
  for (i = -_pH / 2; i <= _pH / 2; ++i)
    for (j = -_pW / 2; j <= _pW / 2; ++j)
    {
      /* vide, pas de bloc mur */
      if (_plat[(i + _pH / 2) * _pW + j + _pW / 2] == 0)
        continue;
      memcpy(nmv, mvMat, sizeof nmv);  /* copie mvMat dans nmv */
      translate(nmv, 2 * j, 0, 2 * i); /* pourquoi *2 ? */
      translate(nmv, 0.0f, 1.0f, 0.0f);
      transform_n_raster(_cube, nmv, projMat);
    }
  /* la balle est scalée, relevée (+1 en y) et placée à sa position (x, y) en (x, z) */
  memcpy(nmv, mvMat, sizeof nmv); /* copie mvMat dans nmv */
  translate(nmv, _paddle_data.x, _paddle_data.y, _paddle_data.z);
  scale(nmv, _paddle_data.w, 1.0f, 1.0f);
  transform_n_raster(_paddle, nmv, projMat);

  memcpy(nmv, mvMat, sizeof nmv); /* copie mvMat dans nmv */
  translate(nmv, _ball.x, 0, _ball.y);
  translate(nmv, 0.0f, 1.0f, 0.0f);
  scale(nmv, _ball.z, _ball.z, _ball.z);
  transform_n_raster(_sphere, nmv, projMat);

  display_bricks(mvMat,projMat,nmv);
  /* déclarer qu'on a changé (en bas niveau) des pixels du screen  */
  gl4dpScreenHasChanged();
  /* fonction permettant de raffraîchir l'ensemble de la fenêtre*/
  gl4dpUpdateScreen(NULL);
}

void parsing()
{
  char *line_to_parse = calloc(1000, sizeof(char));
  char *nb_to_parse = calloc(1000, sizeof(char));
  assert(line_to_parse);
  assert(nb_to_parse);

  FILE *brick_file = fopen("bricks", "r");
  assert(brick_file);
  fgets(line_to_parse, 1000, brick_file);
  fgets(line_to_parse, 1000, brick_file);
  fgets(line_to_parse, 1000, brick_file);

  while (feof(brick_file) == 0)
  {
  }
}

static void display_bricks(float mvMat[16], float projMat[16], float nmv[16])
{
  for (cell_t *encours = _bricks->premier; encours != NULL; encours = encours->next)
  {
    brick_t *brick = encours->data;
    vec3 brick_coords = brick->brick_coords;
    vec3 brick_size = brick->brick_size;
    memcpy(nmv, mvMat, sizeof nmv);
    translate(nmv, brick_coords.x, brick_coords.y, brick_coords.z);
    scale(nmv, brick_size.x, brick_size.y, brick_size.z);
    transform_n_raster(_sphere, nmv, projMat);
  }
}

/*!\brief à appeler à la sortie du programme. */
void sortie(void)
{
  /* on libère nos surfaces */
  if (_quad)
  {
    freeSurface(_quad);
    _quad = NULL;
  }
  if (_cube)
  {
    freeSurface(_cube);
    _cube = NULL;
  }
  if (_paddle)
  {
    freeSurface(_paddle);
    _paddle = NULL;
  }
  if (_sphere)
  {
    freeSurface(_sphere);
    _sphere = NULL;
  }
  /* on libère le plateau */
  free(_plat);
  /* libère tous les objets produits par GL4Dummies, ici
   * principalement les screen */
  gl4duClean(GL4DU_ALL);
}

void print_objects()
{
  printf("Ball Coords X:%f,Y:%f,Z:%f\n", _ball.x, _ball.y, _ball.z);
  printf("Paddle Coords X:%f,Y:%f,Z:%f\n", _paddle_data.x, _paddle_data.y, _paddle_data.z);
}