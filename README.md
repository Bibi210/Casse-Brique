# Brick Destroyer

Un petit casse brique fait maison

## Installer le jeu

`git clone https://code.up8.edu/BrahimaDibassi/brick-destroyer.git`
`make`
`brick_destroyer nb_lvl_custom`
    Par defaut nb_lvl_custom est a 3

## Commande du jeu 
    "Azerty"
    Gauche : Q/Left_Arrow
    Droite : D/Left_Arrow
    1 : Base Cam Angle
    2 : Other Cam Angle

## Custom LvL Format
    Format [Coords][X;Y;Z;]:[Scale][X;Y;Z;][ObjectType][L;]
    Exemple [1.5;2.6;1]:[1;1;1;][2;] 
    Il est possible de doubler les briques en laissant un espace entre deux briques
    Il faut finir le fichier a la derniere brique

ObjectType: 
  - Effect 1 Reverse need cam modif
  - Effect 9 Small Bricks
  - Effect 10 Big pad
  - Effect 11 Big Ball
  - Effect 12 Ball Reset
  - Effect 20 Big Bricks

## Todo List

- [X] Paddle movement
- [X] Paddle Collision
- [X] Bricks Spawning
- [X] Bricks Collision
- [X] LvL Loading from files
- [X] LvL Rand-Gen Loading
- [X] Special Objects
- [X] Changer d'angle de vue
- [X] Death on ground
- [X] Custom Lvl Easy loading
- [X] Slight Improve Overall Visual
- [X] Improve Overall Collision

