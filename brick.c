#include <assert.h>
#include <time.h>

//TODO Print Txt
//TODO Special Object

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
static void keydown_func(int);

static unsigned int _nb_lvl = 0;
static unsigned int _nb_custom_lvl = 3;
/*!\brief une surface représentant un quadrilatère */
static surface_t *_quad = NULL;
/*!\brief une surface représentant un cube */
static surface_t *_cube = NULL;
/*!\brief une surface représentant un Pad */
static surface_t *_paddle = NULL;
/*!\brief une surface représentant une sphère (pour la balle) */
static surface_t *_sphere = NULL;

//* Liste contenant mes objects et mes briques
static liste_t *_bricks = NULL;
static liste_t *_objects = NULL;

/*******Couleur*********/
static vec4 r = {1.0f, 0.0f, 0.0f, 1}, g = {0.0f, 1.0f, 0.0f, 1}, b = {0.0f, 0.0f, 1.0f, 1};
static vec4 d = {0.0f, 0.0f, 0.0f, 1};
static vec4 enemy_buff = {102, 51, 0, 1}, ally_buff = {102, 178, 255, 1};
/*******Couleur*********/

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
/*!\brief ma balle est codée sur un vec4, x y z et w est son rayon
 * son rayon */
static vec4 _ball = {0, -_pH + 5, 0, 0.84f};
/*!\brief ma balle a une vitesse, x est vx, y est vz */
static vec3 _ballv = {0, 0, 0};

//* Un objet a une vitesse des coordonnée un code ,une surface et une taille fixe
static float _obj_size = 0.6;
typedef struct obj_t
{
  surface_t *object_surface;
  vec3 object_coords;
  vec3 object_speed;
  unsigned int object_code;
} obj_t;

//* Une brique a une vitesse des coordonnée et une surface
typedef struct brick_t
{
  surface_t *brick_surface;
  vec3 brick_coords;
  vec3 brick_size;
  unsigned int object_code;
} brick_t;

/*!\brief Etat de la caméra*/
static int _locked_cam = False;

//**** Game Related Functions*****//
static void collision_tests();
static void parsing();
static void next_lvl();
static void reset_ball();
static void reset_pad();
static int random_sign(int x);
static void gen_lvl();
static void brick_free(brick_t *);
static void obj_free(obj_t *);
static void obj_spawn(unsigned, vec3);
static vec4 add_color(vec4, vec4, vec4);
static void apply_obj(unsigned int obj_code);
static void reset_obj();
static void reduce_bricks();
static void big_bricks();
static void mouvement(double);
//**** Game Related Functions*****//

/*!\brief Données du pad sa taille et ses coordonnée */
static vec4 _paddle_data = {0, -_pH + 3, 1, 3.1f};
/*!\brief Marge de mouvement du pad */
static float _mouv_coord = 0.80f;

//****Info de la camera ****//
static float _zoom_out = 30;
static float _angle = -1;
//****Info de la camera ****//

int main(int argc, char **argv)
{

  if (!gl4duwCreateWindow(argc, argv,
                          "Brick Destroyer", /* titre */
                          10, 10, _wW, _wH,
                          GL4DW_SHOWN))
  {
    /* ici si échec de la création */
    return 1;
  }
  /* Pour forcer la désactivation de la synchronisation verticale */
  SDL_GL_SetSwapInterval(0);
  init();

  gl4dpInitScreen();
  gl4duwKeyDownFunc(keydown_func);
  gl4duwIdleFunc(simu);
  gl4duwDisplayFunc(draw);
  gl4duwMainLoop();
  return 0;
}

/*!\brief init de nos données,*/
void init(void)
{
  _bricks = list_init(sizeof(brick_t), (void *)brick_free);
  _objects = list_init(sizeof(brick_t), (void *)obj_free);
  //ANCHOR couleurs des element
  /* on créé nos 3 types de surfaces */
  //ANCHOR Interieur du casse brique
  _quad = mkQuad(); /* ça fait 2 triangles        */
  // ANCHOR Bordure du casse brique
  _cube = mkCube(); /* ça fait 2x6 triangles      */
  // ANCHOR Le pad de jeu
  _paddle = mkCube(); /* ça fait 2x6 triangles      */
  // ANCHOR balle
  _sphere = mkSphere(12, 4); /* ça fait 2x12x5 triangles   */
  /* on change les couleurs de surfaces */
  _sphere->dcolor = r;
  _quad->dcolor = add_color(g, r, d);
  _quad->dcolor.z = 10;
  _cube->dcolor = b;
  _paddle->dcolor = enemy_buff;
  /* on leur rajoute la texture (cube et quad) */
  setTexId(_quad, getTexFromBMP("images/sol.bmp"));
  setTexId(_cube, getTexFromBMP("images/brick.bmp"));
  /* on active l'utilisation de la texture pour les deux */
  enableSurfaceOption(_quad, SO_USE_TEXTURE);
  enableSurfaceOption(_cube, SO_USE_TEXTURE);
  /* on active l'ombrage */
  enableSurfaceOption(_quad, SO_USE_LIGHTING);
  enableSurfaceOption(_cube, SO_USE_LIGHTING);
  enableSurfaceOption(_sphere, SO_USE_LIGHTING);

  _plat = plateau(_pW, _pH);
  /* initialiser aléatoirement la vitesse de la balle (au pifomètre) */

  /* mettre en place la fonction à appeler en cas de sortie */
  atexit(sortie);
}

//*Fonction de gestion des keyboard inputs
static void keydown_func(int keypressed)
{
  if (_angle < 0)
  {
    _locked_cam = False;
  }
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
    break;
  case GL4DK_SPACE:
    if (_ballv.y == 0)
    {
      _ballv.x = 0;
      _ballv.y = _nb_lvl + 28;
    }
    break;
  case GL4DK_1:
    if (_locked_cam == False)
    {
      _zoom_out = 30;
      _angle = -1;
    }
    break;
  case GL4DK_2:
    if (_locked_cam == False)
    {
      _angle = -18;
      _zoom_out = 33;
    }
    break;
  default:
    break;
  }
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
  mouvement(dt);
}

/*!\brief la fonction appelée à chaque display. */
void draw(void)
{
  if (_bricks->list_size == 0)
  {
    list_free(_objects);
    _objects = list_init(sizeof(obj_t), (void *)obj_free);
    next_lvl();
  }
  int i, j;
  float mvMat[16], projMat[16], nmv[16];
  /* effacer l'écran et le buffer de profondeur */
  gl4dpClearScreen();
  clearDepth();
  /* charger un frustum dans projMat */
  MFRUSTUM(projMat, -0.05f, 0.05f,
           -0.05f * _wH / _wW /* pour garder le ratio */,
           0.05f * _wH / _wW /* pour garder le ratio */,
           0.1f, 1000.0f);
  /* charger la matrice identité dans model-view */
  MIDENTITY(mvMat);
  /* on place la caméra */
  lookAt(mvMat, 0, _zoom_out, _angle, 0, 0, 0, 0, 1, 0);

  /* le quadrilatère est agrandi et couché */
  memcpy(nmv, mvMat, sizeof nmv);
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
      memcpy(nmv, mvMat, sizeof nmv);
      translate(nmv, 2 * j, 0, 2 * i);
      translate(nmv, 0.0f, 1.0f, 0.0f);
      transform_n_raster(_cube, nmv, projMat);
    }

  //*Placement du pad
  memcpy(nmv, mvMat, sizeof nmv);
  translate(nmv, _paddle_data.x, _paddle_data.z, _paddle_data.y);
  scale(nmv, _paddle_data.w, 1.0f, 1.0f);
  transform_n_raster(_paddle, nmv, projMat);

  /* la balle est scalée, relevée (+1 en y) et placée à sa position */
  memcpy(nmv, mvMat, sizeof nmv);
  translate(nmv, _ball.x, 0, _ball.y);
  translate(nmv, 0.0f, 1.0f, 0.0f);
  scale(nmv, _ball.w, _ball.w, _ball.w);
  transform_n_raster(_sphere, nmv, projMat);

  //*Placement de toutes les briques
  for (cell_t *encours = _bricks->premier; encours != NULL; encours = encours->next)
  {
    brick_t *brick = encours->data;
    vec3 brick_coords = brick->brick_coords;
    vec3 brick_size = brick->brick_size;

    memcpy(nmv, mvMat, sizeof nmv);
    translate(nmv, brick_coords.x, brick_coords.z, brick_coords.y);
    scale(nmv, brick_size.x, brick_size.z, brick_size.y);
    transform_n_raster(brick->brick_surface, nmv, projMat);
  }
  //*Placement de tout les objets
  for (cell_t *encours = _objects->premier; encours != NULL; encours = encours->next)
  {
    obj_t *obj = encours->data;
    vec3 obj_coords = obj->object_coords;

    memcpy(nmv, mvMat, sizeof nmv);
    translate(nmv, obj_coords.x, obj_coords.z, obj_coords.y);
    scale(nmv, _obj_size, _obj_size, _obj_size);
    transform_n_raster(obj->object_surface, nmv, projMat);
  }

  /* déclarer qu'on a changé (en bas niveau) des pixels du screen  */
  gl4dpScreenHasChanged();
  /* fonction permettant de raffraîchir l'ensemble de la fenêtre*/
  gl4dpUpdateScreen(NULL);
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
  /* On libére nos listes */
  list_free(_bricks);
  list_free(_objects);

  /* on libère le plateau */
  free(_plat);
  /* libère tous les objets produits par GL4Dummies, ici
   * principalement les screen */
  gl4duClean(GL4DU_ALL);
}

//*Lecture d'un fichier niveau
void parsing()
{
  int size_test = 0;
  int next_champ = 0;
  brick_t *br = malloc(sizeof(brick_t));
  char filename[100];
  sprintf(filename, "brick_levels/bricks_lvl%u", _nb_lvl);

  char line_to_parse[1000];
  char nb_to_parse[1000];
  FILE *brick_file = fopen(filename, "r");

  assert(brick_file);
  assert(br);

  while (feof(brick_file) == 0)
  {
    br->object_code = 0;
    char *throw = fgets(line_to_parse, 1000, brick_file);
    throw = throw;
    for (int i = 0, t = 0; line_to_parse[i] != '\0'; i++)
    {
      if (line_to_parse[i] == ';')
      {
        nb_to_parse[t] = '\0';
        float nb = strtof(nb_to_parse, NULL);
        t = 0;
        if (size_test == 0)
        {
          switch (next_champ)
          {
          case 0:
            br->brick_coords.x = nb;
            next_champ++;
            break;
          case 1:
            br->brick_coords.y = nb;
            next_champ++;
            break;
          case 2:
            br->brick_coords.z = nb;
            size_test++;
            next_champ = 0;
            break;
          default:
            break;
          }
        }
        else
        {
          switch (next_champ)
          {
          case 0:
            br->brick_size.x = nb;
            next_champ++;
            break;
          case 1:
            br->brick_size.y = nb;
            next_champ++;
            break;
          case 2:
            br->brick_size.z = nb;
            next_champ++;
            break;
          case 3:
            br->object_code = nb;
            next_champ = 0;
            size_test = 0;
            break;
          default:
            break;
          }
        }
      }
      else if ((line_to_parse[i] >= '0' && line_to_parse[i] <= '9') || line_to_parse[i] == '.' || line_to_parse[i] == '-')
      {
        nb_to_parse[t] = line_to_parse[i];
        t++;
      }
    }
    //*Ajout a la liste de briques
    list_push(_bricks, br);
  }
}

//Applique les deplacement a tout les elements du jeu
static void mouvement(double dt)
{
  /* physique de base : mécanique newtonienne */
  _ball.x += _ballv.x * dt;
  _ball.y += _ballv.y * dt;
  _ball.z += _ballv.z * dt;
  for (cell_t *encours = _objects->premier; encours != NULL; encours = encours->next)
  {
    obj_t *obj = encours->data;
    vec3 obj_speed = obj->object_speed;
    obj->object_coords.x += obj_speed.x * dt;
    obj->object_coords.y += obj_speed.y * dt;
    obj->object_coords.z += obj_speed.z * dt;
  }
}

//* Teste toutes les collision du jeu et leurs concéquences
void collision_tests()
{
  //* Verifie la position du pad
  if (_paddle_data.x - _paddle_data.w <= -_pW + 2.0f)
  {
    _paddle_data.x += _mouv_coord;
  }
  if (_paddle_data.x + _paddle_data.w >= _pW - 2.0f)
  {
    _paddle_data.x -= _mouv_coord;
  }

  //* Verifie la position de la balle vis a vis du pad
  if (_ball.x - _ball.w >= _paddle_data.x - (_paddle_data.w + 1) && _ball.x + _ball.w <= _paddle_data.x + (_paddle_data.w + 1))
  {
    if (_ball.y - _ball.w <= _paddle_data.y)
    {
      _ball.y += 0.15f;
      _ballv.y = -_ballv.y;
      if (_ball.x - _ball.w >= _paddle_data.x - 1.5 && _ball.x + _ball.w <= _paddle_data.x + 1.5)
      {
        _ballv.x = 0;
      }
      else if (_ball.x - _ball.w >= _paddle_data.x - (_paddle_data.w + 1) / 3)
      {
        _ballv.x = 12;
      }
      else if (_ball.x + _ball.w <= _paddle_data.x + (_paddle_data.w + 1) / 3)
      {
        _ballv.x = -12;
      }
    }
  }

  float lg = -_pW + 2.0f + _ball.w; /* limite gauche */
  float ld = _pW - 2.0f - _ball.w;  /* limite droite */
  float lb = _pH - 2.0f - _ball.w;  /* limite basse */
  float lh = -_pH + 2.0f + _ball.w; /* limite haute */

  //* Verifie la position de la balle vis a vis des murs
  if (_ball.x <= lg)
  {
    _ball.x += 0.15f;
    _ballv.x = -_ballv.x;
  }
  else if (_ball.x >= ld)
  {
    _ball.x -= 0.15f;
    _ballv.x = -_ballv.x;
  }
  else if (_ball.y <= lh)
  {
    _ballv.y = -_ballv.y;
    _ball.x -= 0.15f;
  }
  //* Remise a 0 du jeu si l'on touche le sol
  else if (_ball.y < _paddle_data.y)
  {
    _nb_lvl = 0;
    list_free(_bricks);
    list_free(_objects);
    _bricks = list_init(sizeof(brick_t), (void *)brick_free);
    _objects = list_init(sizeof(obj_t), (void *)obj_free);
  }
  else if (_ball.y >= lb)
  {
    _ballv.y = -_ballv.y;
    _ball.x += 0.15f;
  }
  unsigned long long i = 0;

  //* Verifie la position des objets
  for (cell_t *encours = _objects->premier; encours != NULL; encours = encours->next, i++)
  {
    obj_t *obj = encours->data;
    vec3 obj_coords = obj->object_coords;
    unsigned int object_code = obj->object_code;
    if (obj_coords.y - _obj_size <= _paddle_data.y)
    {
      list_del_at(_objects, i);
      if (obj_coords.x - _obj_size >= _paddle_data.x - (_paddle_data.w + 1) && obj_coords.x + _obj_size <= _paddle_data.x + (_paddle_data.w + 1))
      {
        apply_obj(object_code);
      }
      break;
    }
  }

  i = 0;
  //*  //* Verifie la position de la balle vis a vis des briques
  for (cell_t *encours = _bricks->premier; encours != NULL; encours = encours->next, i++)
  {
    brick_t *br = encours->data;
    if (_ball.x + _ball.w >= br->brick_coords.x - br->brick_size.x && _ball.x - _ball.w <= br->brick_coords.x + br->brick_size.x)
    {
      if (_ball.y - _ball.w <= br->brick_coords.y && _ball.y + _ball.w >= br->brick_coords.y - br->brick_size.y)
      {
        _ballv.x = -_ballv.x;
        _ballv.y = -_ballv.y;
        obj_spawn(br->object_code, br->brick_coords);
        list_del_at(_bricks, i);
        return;
      }
    }
  }
}

//*Passe au niveau suivant du jeu
static void next_lvl()
{
  reset_obj();
  reset_ball();
  reset_pad();
  _nb_lvl++;
  if (_nb_lvl < _nb_custom_lvl)
  {

    parsing();
  }
  else
  {
    gen_lvl();
  }

  for (cell_t *encours = _bricks->premier; encours != NULL; encours = encours->next)
  {
    brick_t *une_brique = encours->data;
    une_brique->brick_surface = mkCube();
    setTexId(une_brique->brick_surface, getTexFromBMP("images/tijolinho.bmp"));
    if (rand() <= RAND_MAX / 2)
    {
      une_brique->brick_surface->dcolor = add_color(g, r, d);
    }
    else
    {
      une_brique->brick_surface->dcolor = add_color(g, r, b);
    }

    enableSurfaceOption(une_brique->brick_surface, SO_USE_LIGHTING);
    enableSurfaceOption(une_brique->brick_surface, SO_USE_TEXTURE);
  }
}

//*Replace la ball au centre
static void reset_ball()
{
  _ball.w = 0.84f;
  _ballv.x = 0;
  _ballv.y = 0;
  _ball.x = 0;
  _ball.y = -_pH + 5;
}

//*Replace le pad au centre
static void reset_pad()
{
  _paddle_data.x = 0;
  _paddle_data.y = -_pH + 3;
  _paddle_data.z = 1;
  _paddle_data.w = 3.1f;
}

//* Genere aléatoirement un niveau
static void gen_lvl()
{
  brick_t *une_brique = malloc(sizeof(brick_t));

  unsigned int rand_object = 0;
  for (unsigned int nb_brick = _nb_lvl * 2; nb_brick > 0; nb_brick--)
  {
    rand_object = 0;
    une_brique->brick_coords.x = random_sign(rand() % _pW - 6) + 1;
    une_brique->brick_coords.y = random_sign(rand() % _pH - 4) + 1;

    while (une_brique->brick_coords.y <= -_pH + 10)
    {
      une_brique->brick_coords.y++;
    }

    une_brique->brick_coords.z = 1;

    une_brique->brick_size.x = (rand() % 2) + 1;
    une_brique->brick_size.y = (rand() % 2) + 1;
    une_brique->brick_size.z = (rand() % 3) + 1;

    if (nb_brick < 6)
    {
      while (rand_object != 1 && rand_object != 9 && rand_object != 10 &&
             rand_object != 11 &&
             rand_object != 12 &&
             rand_object != 20)
      {
        rand_object = rand() % 21;
      }
    }
    une_brique->object_code = rand_object;
    list_push(_bricks, une_brique);
  }
  free(une_brique);
}

static int random_sign(int x)
{
  return rand() <= RAND_MAX / 2 ? x : -x;
}

//* Libére la structure brick_t
static void brick_free(brick_t *brick)
{
  if (brick->brick_surface)
  {
    freeSurface(brick->brick_surface);
  }
}

//* Libére la structure obj_t
static void obj_free(obj_t *obj)
{
  if (obj->object_surface)
  {
    freeSurface(obj->object_surface);
  }
}

//* Apparition d'un objet dans le jeu
static void obj_spawn(unsigned int code, vec3 spawn_coord)
{
  if (code == 0)
  {
    return;
  }

  obj_t *obj = malloc(sizeof(obj_t));
  obj->object_code = code;
  obj->object_coords = spawn_coord;
  obj->object_surface = mkSphere(12, 4);

  if (code <= 9)
  {
    obj->object_surface->dcolor = enemy_buff;
    obj->object_speed.y = -12;
  }
  else
  {
    obj->object_surface->dcolor = ally_buff;
    obj->object_speed.y = -18;
  }
  list_push(_objects, obj);
  free(obj);
}

static vec4 add_color(vec4 color1, vec4 color2, vec4 color3)
{
  color1.x += color2.x + color3.x;
  color1.y += color2.y + color3.y;
  color1.z += color2.z + color3.z;
  return color1;
}

//* Applique les effet d'un objet sur le jeu
static void apply_obj(unsigned int code)
{
  //* Effect 1 Reverse need cam modif
  //* Effect 9 Small Bricks

  //* Effect 10 Big pad
  //* Effect 11 Big Ball
  //* Effect 12 Ball Reset
  //* Effect 20 Big Bricks

  switch (code)
  {
  case 1:
    _angle = -_angle;
    _locked_cam = True;
    break;
  case 9:
    reduce_bricks();
    break;
  case 10:
    if (_paddle_data.x + _paddle_data.w + 2 < _pW)
    {
      _paddle_data.w += 2;
    }
    break;
  case 11:
    _ball.w += 0.7;
    break;
  case 12:
    _ballv.x = 0;
    _ballv.y = 0;
    _ball.x = 0;
    _ball.y = -_pH + 5;
    break;
  case 20:
    big_bricks();
    break;
  default:
    break;
  }
}

//* Annule tout les effet
static void reset_obj()
{
  _locked_cam = False;
  _paddle_data.x = 3;
  _ball.w = 0.84f;
  if (_angle > 0)
  {
    _angle = -1;
  }
}

static void reduce_bricks()
{
  for (cell_t *current = _bricks->premier; current != NULL; current = current->next)
  {
    brick_t *brick = current->data;
    if (brick->brick_size.x > 1)
    {
      brick->brick_size.x--;
    }
    if (brick->brick_size.y > 1)
    {
      brick->brick_size.y--;
    }
    if (brick->brick_size.y > 1)
    {
      brick->brick_size.y--;
    }
  }
}

static void big_bricks()
{
  for (cell_t *current = _bricks->premier; current != NULL; current = current->next)
  {
    brick_t *brick = current->data;
    if (abs((int)brick->brick_coords.x) + brick->brick_size.x + 3 < _pW)
    {
      brick->brick_size.x++;
    }
    if (abs((int)brick->brick_coords.y) + brick->brick_size.y + 3 < _pH)
    {
      brick->brick_size.y++;
    }
    brick->brick_size.z++;
  }
}