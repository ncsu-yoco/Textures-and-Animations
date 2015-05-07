//
//  main.cpp
//  Pg2_opengl_kmsomani
//
//  Created by Kapil Somani on 10/2/14.
//  Copyright (c) 2014 NCSU_Graphics. All rights reserved.
//

#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <iostream>
#include<math.h>
#include<string.h>
#include<fstream>
#include<dirent.h>
#include<unistd.h>

using namespace std;

#define SMALL_NUM 0.00000001
#define maxVertex 100000
#define maxFace 100000
#define maxGroup 100000
#define maxLight 10
#define PI 3.142

string folderLoc = "";
string objFol = "inputs/";
string inputObj = "al.obj";
string texImg = "niagra.bmp";


unsigned char* data;
unsigned int width, height;
static GLuint texName;

//Default values for few parameters
float eye[3]={00,00,300};
float lookAt[3]={100,100,100};
float viewUp[3]={1,0,1};
int xPix=600;
int yPix=600;

int vrtxCnt;    //stores vertex count
int faceCnt;    //stores face count
int grupCnt;    //stores group count
int currGrup;
int mtrlCnt;    //stores material count
int noLight;
float vertices[maxVertex][3];
int faces[maxFace][40];
string groups[maxGroup][2];
string material[maxGroup];
GLfloat mtrlConst[maxGroup][10];
double viewPort[3];
float *light;

int xWid,yWid;  //dimensions of viewPort in world co-ordinated
string inpWin;
string inpViw;
string inpLig;

string objFiles[30];
string texFiles[30];
int objCount=0;
int texCount=0;

float clampAngle(float ang)
{
    if (ang < 0)
        return 0;
    if (ang > 180)
        return 180;
    else
        return ang;
}

void display(void)
{
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glColor3f (0.0, 1.0, 0.0);
    
    glEnable(GL_TEXTURE_2D);
//    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,GL_DECAL);
    glBindTexture(GL_TEXTURE_2D, texName);
    
    int i;
    for (i=0;i<faceCnt;i++)
    {
        int k;
        for (k=0;k<mtrlCnt;k++)
        {
            if( groups[faces[i][1]][0] == material[k] )
            {
                break;
            }
        }
        
        GLfloat mat_amb[] = {mtrlConst[k][0],mtrlConst[k][1],mtrlConst[k][2]};
        GLfloat mat_diff[] = {mtrlConst[k][3],mtrlConst[k][4],mtrlConst[k][5]};
        GLfloat mat_specular[] = {mtrlConst[k][6],mtrlConst[k][7],mtrlConst[k][8]};
        
        glMaterialfv(GL_FRONT,GL_AMBIENT,mat_amb);
        glMaterialfv(GL_FRONT,GL_DIFFUSE, mat_diff);
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
        
        glMaterialfv(GL_BACK,GL_AMBIENT,mat_amb);
        glMaterialfv(GL_BACK,GL_DIFFUSE, mat_diff);
        glMaterialfv(GL_BACK, GL_SPECULAR, mat_specular);
        
        float thx,thy,dth;  //to determine texture co-ordinates
        float tx,ty;
        thx=0;
        thy=0;
        tx=0;
        ty=0;
        dth=(4*180)/(2*faces[i][0]);
        
        glBegin(GL_POLYGON);
        int j;
        
        for (j=2;j<faces[i][0]+2 ;j++)
        {
            if ( (j-2 >= faces[i][0]/2) && (dth > 0) )
            {
                dth = -dth;
            }
            if ( (j-2 < faces[i][0]/4) || ( (j-2>= faces[i][0]/2) && (j-2 < faces[i][0]*3/4) ) )
            {
                ty =  sinf(thy*PI/180);
                tx =  sinf(thx*PI/180);
                thx = thx + dth;
            }
            if ( ( (j-2>= faces[i][0]/4) && (j-2 < faces[i][0]/2) )|| (j-2 >= faces[i][0]*3/4) )
            {
                tx =  sinf(thx*PI/180);
                ty =  sinf(thy*PI/180);
                thy = thy + dth;
            }
            thx=clampAngle(thx);
            thy=clampAngle(thy);
            glTexCoord2f(tx,ty);
            glVertex3f(vertices[faces[i][j]-1][0],vertices[faces[i][j]-1][1],vertices[faces[i][j]-1][2]);
        }
        glEnd();
    }
    glFlush ();
    glutSwapBuffers();
}

void init (void)
{
    /*  select clearing (background) color       */
    // glClearColor (1.0, 1.0, 1.0, 1.0);
    //glClearColor (0.0, 0.0, 0.0, 0.0);
    glClearColor (0.3, 0.3, 0.3, 1.0);
    /*  initialize viewing values  */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //  glOrtho(-500,500,-500,500,500,-500);
    glOrtho(-xPix/2, xPix/2, -yPix/2, yPix/2, (xPix+yPix)/4 , -(xPix+yPix)/4 );
    gluLookAt(eye[0],eye[1],eye[2],lookAt[0],lookAt[1],lookAt[1],viewUp[0],viewUp[1],viewUp[2]);
    //  gluOrtho2D(-xPix/2.0,xPix/2.0,-yPix/2.0,yPix/2.0);
    
    glShadeModel (GL_SMOOTH);
    
    GLfloat *light_position;
    light_position = static_cast<GLfloat*>(light);
    
    glEnable(GL_LIGHTING);
    GLenum lightptr;
    lightptr = GL_LIGHT0;
    GLfloat param[3]={0.3,0.3,0.3};
    int i=0;
    while (i<7 && i<noLight) {
        glLightfv(lightptr,GL_POSITION,light_position);
        glLightfv(lightptr,GL_AMBIENT,param);
        glLightfv(lightptr,GL_DIFFUSE,param);
        glLightfv(lightptr,GL_SPECULAR,param);
        glEnable(lightptr);
        light_position=light_position+3;
        lightptr++;
        i++;
    }
    glEnable(GL_DEPTH_TEST);
}

void setTex()
{
    string path;
    path = folderLoc + objFol + texImg;
    // Data read from the header of the BMP file
    unsigned char header[54]; // Each BMP file begins by a 54-bytes header
    unsigned int dataPos;     // Position in the file where the actual data begins
    unsigned int imageSize;   // = width*height*3
    // Open the file in binary mode
    FILE* fp = fopen(path.c_str(),"rb");
    if (!fp)
    {
        cout<<"Image could not be opened\n";
        return ;
    }
    if ( fread(header, 1, 54, fp)!=54 )
    {
        // If not 54 bytes read : problem
        cout<<"Not a correct BMP file\n";
        return ;
    }
    if ( header[0]!='B' || header[1]!='M' )
    {
        cout<<"Not a correct BMP file\n";
        return ;
    }
    // Read ints from the byte array
    dataPos = *(int*)&(header[0x0A]);
    imageSize = *(int*)&(header[0x22]);
    width = *(int*)&(header[0x12]);
    height = *(int*)&(header[0x16]);
    // Some BMP files are misformatted, guess missing information
    if (imageSize==0)
        imageSize=(width)*(height)*3; // 3 : one byte for each Red, Green and Blue component
    if (dataPos==0)
        dataPos=54; // The BMP header is done
    // Create a buffer
    data = new unsigned char [imageSize];
    // Read the actual data from the file into the buffer
    if (fread(data,1,imageSize,fp) != imageSize)
    {
        fclose(fp);
        return ;
    }
    //Everything is in memory now, the file can be closed
    fclose(fp);
    
    glShadeModel(GL_FLAT);	//to prevent color interpolation
    glEnable(GL_DEPTH_TEST);
    //specify pixels are byte-aligned while unpacking from memory
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &texName);//generate
    glBindTexture(GL_TEXTURE_2D, texName);//bind to a 2-D texture
    //wrap the image along s coordinate (width)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //wrap the image along t coordinate (height)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //how to treat magnification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //how to treat de-magnification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //bmp stores BGR instead of RGB
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, data);
}

void reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(120,w/h,1,30000);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(eye[0],eye[1],eye[2],lookAt[0],lookAt[1],lookAt[2],viewUp[0],viewUp[1],viewUp[2]);
}

void redraw(int value)
{
    //we aren't doing anything with value here
    glutPostRedisplay();
    glutTimerFunc(10,redraw,0);
}

void keyboard (unsigned char key, int x, int y)
{
    switch (key)
    {
        case 27:
            exit(0);
            break;
        case 91:   //character [    -   along +Z
            glTranslatef(0,0,5);
            break;
        case 93:   //character ]    - along -Z
            glTranslatef(0,0,-5);
            break;
        case 122:   //character z   -   zoom in
            glScalef(1.1,1.1,1.1);
            break;
        case 120:   //character x   -   zoom out
            glScalef(0.9,0.9,0.9);
            break;
        case 116:   //character t   -   rotate clockwise along X
            //cout<<"RO C";
            glRotatef(-10,1,0,0);
            break;
        case 121:   //character y   -   rotate counter clockwise along X
            //  cout<<"RO CC";
            glRotatef(10,1,0,0);
            break;
        case 101:   //character e   -   rotate clockwise along Y
            //cout<<"RO C";
            glRotatef(-10,0,1,0);
            break;
        case 114:   //character r   -   rotate counter clockwise along Y
            //  cout<<"RO CC";
            glRotatef(10,0,1,0);
            break;
        case 113:   //character q   -   rotate clockwise along Z
            //cout<<"RO C";
            glRotatef(-10,0,0,1);
            break;
        case 119:   //character w   -   rotate counter clockwise along Z
            //  cout<<"RO CC";
            glRotatef(10,0,0,1);
            break;
        default:
            break;
    }
    glutTimerFunc(60,redraw,0);
}

void specialKeys(int key,int x,int y)
{
    switch(key)
    {
        case GLUT_KEY_UP:   // along +Y
            glTranslatef(0,5,0);
            break;
        case GLUT_KEY_DOWN: //along -Y
            glTranslatef(0,-5,0);
            break;
        case GLUT_KEY_LEFT: //along -X
            glTranslatef(-5,0,0);
            break;
        case GLUT_KEY_RIGHT:    //along +X
            glTranslatef(5,0,0);
            break;
        default:
            break;
    }
    glutTimerFunc(60,redraw,0);
}


void setScene()
{
    string inpWin = "window.txt";
    string inpViw = "view.txt";
    string inpLig = "lights.txt";
    noLight = 0;
    light=new float[maxLight*3];
    
    string delimiter = " ";
    string line;
    ifstream winFile(folderLoc + inpWin);
    if ( winFile.is_open() )
    {
        while ( winFile.good() )
        {
            getline(winFile,line);
            string token = line.substr(0, line.find(delimiter));
            if ( token[0] == '#' ); //Comments Line
            else if ( token == "xDimension" )
            {
                xPix = atof(line.substr(11,line.length()).c_str());
            }
            else if ( token == "yDimension" )
            {
                yPix = atof(line.substr(11,line.length()).c_str());
            }
        }
    }
    else
    {
        exit (13);
    }
    winFile.close();
    
    ifstream viwFile(folderLoc + inpViw);
    if ( viwFile.is_open() )
    {
        while ( viwFile.good() )
        {
            getline(viwFile,line);
            string token = line.substr(0, line.find(delimiter));
            if (token[0]=='#'); //Comments Line
            else if ( token == "Eye" )
            {
                int i;
                long n;
                line=line.substr(4,line.length());
                i=0;
                while(!line.empty())
                {
                    n=line.find(delimiter);
                    string s;
                    s=line.substr(0,n);
                    line=line.substr(n+1,line.length());
                    eye[i]=atof(s.c_str());
                    i++;
                    if( n < 0 )
                    {
                        break;
                    }
                }
            }
            else if ( token == "ViewUp" )
            {
                int i;
                long n;
                line=line.substr(7,line.length());
                i=0;
                while(!line.empty())
                {
                    n=line.find(delimiter);
                    string s;
                    s=line.substr(0,n);
                    line=line.substr(n+1,line.length());
                    viewUp[i]=atof(s.c_str());
                    i++;
                    if( n < 0 )
                    {
                        break;
                    }
                }
                //cout<<viewUp[0]<<" "<<viewUp[1]<<" "<<viewUp[2];
            }
            else if ( token == "LookAt" )
            {
                int i;
                long n;
                line=line.substr(7,line.length());
                i=0;
                while(!line.empty())
                {
                    n=line.find(delimiter);
                    string s;
                    s=line.substr(0,n);
                    line=line.substr(n+1,line.length());
                    lookAt[i]=atof(s.c_str());
                    i++;
                    if( n < 0 )
                    {
                        break;
                    }
                }
            }
        }
    }
    else
    {
        exit(14);
    }
    viwFile.close();
    
    ifstream winLig(folderLoc + inpLig);
    if ( winLig.is_open() )
    {
        while ( winLig.good() )
        {
            getline(winLig,line);
            string token = line.substr(0, line.find(delimiter));
            if (token[0]=='#'); //Comments Line
            else if ( token == "Light" )
            {
                int i;
                long n;
                line=line.substr(6,line.length());
                i=0;
                while( !line.empty() )
                {
                    n=line.find(delimiter);
                    string s;
                    s=line.substr(0,n);
                    line=line.substr(n+1,line.length());
                    light[noLight*3 + i]=atof(s.c_str());
                    i++;
                    if( n < 0 )
                    {
                        break;
                    }
                }
                noLight++;
            }
        }
    }
    else
    {
        exit(15);
    }
    // cout<<"SetUp file loaded"<<endl;
}


void loadObj()
{
    vrtxCnt = 0;
    faceCnt = 0;
    grupCnt = 0;
    currGrup = 0;
    mtrlCnt = 0;
    string line;
    string matFile = "";
    string objFile = folderLoc + objFol + inputObj;
    ifstream myfile (objFile);
    string delimiter = " ";
    if (myfile.is_open())
    {
        string grp;
        while ( getline(myfile,line) )
        {
            string token = line.substr(0, line.find(delimiter));
            if (token[0] == '#' );  //Comments line
            else if( token == "mtllib" )
            {
                matFile=folderLoc + objFol + line.substr(7,line.length());
            }
            else if( token == "v" )
            {
                int i;
                long n;
                line=line.substr(2,line.length());
                i=0;
                while(!line.empty())
                {
                    n=line.find(delimiter);
                    string s;
                    s=line.substr(0,n);
                    line=line.substr(n+1,line.length());
                    vertices[vrtxCnt][i]=atof(s.c_str());
                    i++;
                    if( n < 0 || i == 3 )
                    {
                        break;
                    }
                }
                vrtxCnt++;
            }
            else if( token == "g" )
            {
                if(line.length() < 2)
                {
                    continue;
                }
                line=line.substr(2,line.length());
                int i;
                for(i=0;i<grupCnt;i++)
                {
                    if(groups[i][0] == line)
                    {
                        break;
                    }
                }
                if(i<grupCnt)
                {
                    currGrup=i;
                }
                else
                {
                    groups[grupCnt][0]=line;
                    currGrup=grupCnt;
                    grupCnt++;
                    
                }
            }
            else if( token == "group" )
            {
                grp=line.substr(6,line.length());
            }
            else if( token == "usemtl" )
            {
                line=line.substr(7,line.length());
                int i;
                for(i=0;i<grupCnt;i++)
                {
                    if( grp == groups[i][0] )
                    {
                        groups[i][1]=line;
                        break;
                    }
                }
            }
            else if( token == "f" )
            {
                int i;
                long n;
                line=line.substr(2,line.length());
                i=2;
                faces[faceCnt][0]=0;
                faces[faceCnt][1]=currGrup;
                while(!line.empty())
                {
                    n=line.find(delimiter);
                    string s;
                    s=line.substr(0,n);
                    line=line.substr(n+1,line.length());
                    faces[faceCnt][i]=atoi(s.c_str());
                    i++;
                    faces[faceCnt][0]++;
                    if( n < 0 )
                    {
                        break;
                    }
                }
                faceCnt++;
            }
        }
        myfile.close();
        if(matFile == "")
        {
            return;
        }
        ifstream matF(matFile);
        if (matF.is_open())
        {
            while ( matF.good() )
            {
                getline(matF,line);
                string token = line.substr(0, line.find(delimiter));
                if (token[0] == '#' );  //Comments line
                else if( token == "newmtl" )
                {
                    material[mtrlCnt]=line.substr(7,line.length());
                    mtrlCnt++;
                }
                else if( token == "Ka" )
                {
                    int i;
                    long n;
                    line=line.substr(3,line.length());
                    i=0;
                    while(!line.empty())
                    {
                        n=line.find(delimiter);
                        string s;
                        s=line.substr(0,n);
                        line=line.substr(n+1,line.length());
                        mtrlConst[mtrlCnt-1][i]=atof(s.c_str());
                        i++;
                        if( n < 0 )		//Line ends
                        {
                            break;
                        }
                    }
                }
                else if( token == "Kd" )
                {
                    int i;
                    long n;
                    line=line.substr(3,line.length());
                    i=3;
                    while(!line.empty())
                    {
                        n=line.find(delimiter);
                        string s;
                        s=line.substr(0,n);
                        line=line.substr(n+1,line.length());
                        mtrlConst[mtrlCnt-1][i]=atof(s.c_str());
                        i++;
                        if( n < 0 )
                        {
                            break;
                        }
                    }
                }
                else if( token == "Ks" )
                {
                    int i;
                    long n;
                    line=line.substr(3,line.length());
                    i=6;
                    while(!line.empty())
                    {
                        n=line.find(delimiter);
                        string s;
                        s=line.substr(0,n);
                        line=line.substr(n+1,line.length());
                        mtrlConst[mtrlCnt-1][i]=atof(s.c_str());
                        i++;
                        if( n < 0 )
                        {
                            break;
                        }
                    }
                }
                else if( token == "N" )
                {
                    string s;
                    s=line.substr(2,line.length());
                    mtrlConst[mtrlCnt-1][9]=atof(s.c_str());
                }
            }
        }
        else
        {
            exit(12);
        }
    }
    else
    {
        exit(11);
    }
}

void objDetails()
{
    cout<<"Vertex Count : "<<vrtxCnt<<endl;
    cout<<"Face Count : "<<faceCnt<<endl;
    cout<<"Group Count : "<<grupCnt<<endl;
    cout<<"Material Count : "<<mtrlCnt<<endl;
    cout<<"xPix: "<<xPix<<" yPix: "<<yPix<<endl;
    cout<<"List of Vertices"<<endl;
    for (int i=0;i<vrtxCnt;i++)
    {
        cout<<"\tV"<<i+1<<" ["<<vertices[i][0]<<","<<vertices[i][1]<<","<<vertices[i][2]<<"]"<<endl;
    }
    cout<<"List of Groups"<<endl;
    for(int i=0;i<grupCnt;i++)
    {
        cout<<"\tG"<<i+1<<" : "<<groups[i][0]<<" M:"<<groups[i][1]<<endl;
    }
    cout<<"List of Faces"<<endl;
    for (int i=0;i<faceCnt;i++)
    {
        int j;
        cout<<"\tF ("<<faces[i][0]<<") ("<<faces[i][1]<<") (";
        for(j=2;j<faces[i][0]+2;j++)
        {
            cout<<faces[i][j]<<" ";
        }
        cout<<")"<<endl;
    }
    cout<<"Details on materials"<<endl;
    for (int i=0;i<mtrlCnt;i++)
    {
        cout<<"\t"<<material[i]<<"("<<mtrlConst[i][0]<<"|"<<mtrlConst[i][1]<<"|"<<mtrlConst[i][2]<<"|"<<mtrlConst[i][3]<<"|"<<mtrlConst[i][4]<<"|"<<mtrlConst[i][5]<<"|"<<mtrlConst[i][6]<<"|"<<mtrlConst[i][7]<<"|"<<mtrlConst[i][8]<<"|"<<mtrlConst[i][9]<<")"<<endl;
    }
}

void myMenu(int id)
{
    id = id - 1;
    if (id < 30 )
    {
        inputObj = objFiles[id];
        loadObj();
    }
    else
    {
        id = id-30;
        texImg = texFiles[id];
        setTex();
    }
}

void setMenu()
{
    
    DIR *dir;
    struct dirent *ent;
    const char* workDir = (folderLoc + objFol).c_str();
    if ((dir = opendir (workDir)) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            string file;
            file = ent->d_name;
            if(file.find(".obj") != string::npos )
            {
                objFiles[objCount++]=file;
            }
            if(file.find(".bmp") != string::npos )
            {
                texFiles[texCount++]=file;
            }
        }
        closedir (dir);
    }
    
    int objMenu = glutCreateMenu(myMenu);
    for (int k=0;k<objCount;k++)
    {
        glutAddMenuEntry(objFiles[k].c_str(), k+1);
    }
    
    int texMenu = glutCreateMenu(myMenu);
    for (int k=0;k<texCount;k++)
    {
        glutAddMenuEntry(texFiles[k].c_str(), k+31);
    }
    
    glutCreateMenu(myMenu);
    glutAddSubMenu("Choose Model",objMenu);
    glutAddSubMenu("Choose Texture",texMenu);
    
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

/*
 *  Declare initial window size, position, and display mode
 *  (single buffer and RGBA).  Open window with "hello"
 *  in its title bar.  Call initialization routines.
 *  Register callback function to display graphics.
 *  Enter main loop and process events.
 */
int main(int argc, char** argv)
{
    
    setScene();
    loadObj();
    // objDetails();
    
    glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize (xPix, yPix);
    glutInitWindowPosition (100, 100);
    glutCreateWindow ("Intro2CG-Assignment II");
    
    init ();
    setTex();
    setMenu();
    
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    
    glutMainLoop();
    
    return 0;   /* ISO C requires main to return int. */
}