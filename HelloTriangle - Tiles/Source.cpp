#include <iostream>
#include <string>
#include <assert.h>
#include <windows.h>  
#include <fstream>
using namespace std;
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Shader.h"
#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

#include "Sprite.h"
#include "Timer.h"

enum directions {NONE = -1, LEFT, RIGHT, UP, DOWN};

// Prot�tipo da fun��o de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Protótipos das funções
GLuint loadTexture(string filePath, int &imgWidth, int &imgHeight);
bool CheckCollision(Sprite &one, Sprite &two);

// Dimens�es da janela (pode ser alterado em tempo de execu��o)
const GLuint WIDTH = 1600, HEIGHT = 1200;

//Variáveis globais

int dir = NONE;
//Variáveis para armazenar as infos do tileset
GLuint tilesetTexID;
glm::vec2 offsetTex; //armazena o deslocamento necessário das coordenadas de textura no tileset
GLuint VAOTile;
int nTiles;
glm::vec2 tileSize;
int pontuacao;
bool moedac1, moedac2, moedac3, moedac4, moedac5;

//Variáveis para armazenar as infos do tilemap
glm::vec2 tilemapSize;
const int MAX_COLUNAS = 15;
const int MAX_LINHAS = 15;
int tilemap[MAX_LINHAS][MAX_COLUNAS]; //este é o mapa de índices para os tiles do tileset

//Função para fazer a leitura do tilemap do arquivo
void loadMap(string fileName);
GLuint setupTile();
void drawMap(Shader &shader);
void desenharMoedas();

//Variáveis para controle de um personagem no mapa
glm::vec2 iPos; //índice do personagem no mapa

glm::vec2 posIni; //posição inicial de desenho do mapa
Sprite player, coin, coin1, coin2, coin3, coin4;
glm::vec2 coinPos = glm::vec2(6, 1);
glm::vec2 coin1Pos = glm::vec2(11, 10);
glm::vec2 coin2Pos = glm::vec2(10, 11);
glm::vec2 coin3Pos = glm::vec2(11, 11);
glm::vec2 coin4Pos = glm::vec2(10, 10);

// Fun��o MAIN
int main()
{
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Desafio GB M6", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);

	// GLAD: carrega todos os ponteiros d fun��es da OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;

	}

	const GLubyte* renderer = glGetString(GL_RENDERER); /* get renderer string */
	const GLubyte* version = glGetString(GL_VERSION); /* version as a string */
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);

	Shader shader("HelloTriangle.vs","HelloTriangle.fs");
	Shader shaderDebug("HelloTriangle.vs","HelloTriangleDebug.fs");
	
	int imgWidth, imgHeight;
	GLuint texID = loadTexture("./george.png", imgWidth, imgHeight);
	

	player.inicializar(texID, 4, 4, glm::vec3(400.0,150.0,0.0), glm::vec3(imgWidth*0.5,imgHeight*0.5,1.0),0.0,glm::vec3(1.0,0.0,1.0));
	player.setShader(&shader);
	player.setShaderDebug(&shaderDebug);

	GLuint texID2 = loadTexture("../Textures/items/Pirate-Stuff/Icon31.png", imgWidth, imgHeight);

	coin.inicializar(texID2, 1, 1, glm::vec3(450.0,700.0,0.0), glm::vec3(imgWidth*0.5,imgHeight*0.5,1.0),0.0,glm::vec3(1.0,0.0,1.0));
	coin.setShader(&shader);
	coin.setShaderDebug(&shaderDebug);
	
	coin1.inicializar(texID2, 1, 1, glm::vec3(450.0,700.0,0.0), glm::vec3(imgWidth*0.5,imgHeight*0.5,1.0),0.0,glm::vec3(1.0,0.0,1.0));
	coin1.setShader(&shader);
	coin1.setShaderDebug(&shaderDebug);

	coin2.inicializar(texID2, 1, 1, glm::vec3(450.0,700.0,0.0), glm::vec3(imgWidth*0.5,imgHeight*0.5,1.0),0.0,glm::vec3(1.0,0.0,1.0));
	coin2.setShader(&shader);
	coin2.setShaderDebug(&shaderDebug);

	coin3.inicializar(texID2, 1, 1, glm::vec3(450.0,700.0,0.0), glm::vec3(imgWidth*0.5,imgHeight*0.5,1.0),0.0,glm::vec3(1.0,0.0,1.0));
	coin3.setShader(&shader);
	coin3.setShaderDebug(&shaderDebug);

	coin4.inicializar(texID2, 1, 1, glm::vec3(450.0,700.0,0.0), glm::vec3(imgWidth*0.5,imgHeight*0.5,1.0),0.0,glm::vec3(1.0,0.0,1.0));
	coin4.setShader(&shader);
	coin4.setShaderDebug(&shaderDebug);

	//Leitura do tilemap
	loadMap("map2.txt");
	VAOTile = setupTile();	

	//Habilita o shader que sera usado (glUseProgram)
	shader.Use();

	glm::mat4 projection = glm::ortho(0.0, 800.0, 600.0, 0.0, -1.0, 1.0);
	//Enviando para o shader via variável do tipo uniform (glUniform....)
	shader.setMat4("projection",glm::value_ptr(projection));

	glActiveTexture(GL_TEXTURE0);
	shader.setInt("texBuffer", 0);

	shaderDebug.Use();
	shaderDebug.setMat4("projection",glm::value_ptr(projection));

	cout << "tilemapSize.x: " << tilemapSize.x << endl;
	cout << "tilemapSize.y: " << tilemapSize.y << endl;
	cout << "tileSize.x: " << tileSize.x << endl;
	cout << "tileSize.y: " << tileSize.y << endl;

	posIni.x = 50; 
	posIni.y = 300;

	iPos.x = 0;
	iPos.y = 0; //defini isso aqui para o personagem começar na minha ilha

	// Loop da aplica��o - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		//timer.start();
		glfwPollEvents();

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //cor de fundo
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		drawMap(shader);

		float c = iPos.x;
		float r = iPos.y;
		// Tamanho dos tiles
		float tw = tileSize.x;
		float th = tileSize.y;

		float x = posIni.x + (c * tw / 2 + r * tw / 2);
		float y = posIni.y + (c * th / 2 - r * th / 2);
		player.setPosicao(glm::vec3(x,y,0.0)); //passa a posição baseada no indice da matriz
		player.desenhar();

		desenharMoedas();

		// Troca os buffers da tela
		glfwSwapBuffers(window);
	}
	
	glfwTerminate();
	return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	// ESQUERDA cima
	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
	{
		int linha = (int)iPos.x;
		int coluna = (int)iPos.y;
		if (tilemap[coluna][linha-1] == 3)
		{
			iPos.x = iPos.x;
			iPos.y = iPos.y;
		}
		else
		{
			dir = LEFT;
			iPos.x -= 1;
			player.moverParaEsquerda();
			if (linha == 6 && coluna == 1)
			{
				moedac1 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}
			if (linha == 11 && coluna == 10)
			{
				moedac2 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}
			if (linha == 10 && coluna == 11)
			{
				moedac3 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}
			if (linha == 11 && coluna == 11)
			{
				moedac4 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}
			if (linha == 10 && coluna == 10)
			{
				moedac5 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}
		}
	}

	// ESQUERDA reto
	if (key == GLFW_KEY_A && action == GLFW_PRESS)
	{
		int linha = (int)iPos.x;
		int coluna = (int)iPos.y;
		if (tilemap[coluna-1][linha-1] == 3)
		{
			iPos.x = iPos.x;
			iPos.y = iPos.y;
		}
		else
		{
			dir = LEFT;
			iPos.x -= 1;
			iPos.y -= 1;
			player.moverParaEsquerda();
			if (linha == 6 && coluna == 1)
			{
				moedac1 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}
			if (linha == 11 && coluna == 10)
			{
				moedac2 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}
			if (linha == 10 && coluna == 11)
			{
				moedac3 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}
			if (linha == 11 && coluna == 11)
			{
				moedac4 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}
			if (linha == 10 && coluna == 10)
			{
				moedac5 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}			
		}
	}

	// ESQUERDA baixo
	if (key == GLFW_KEY_Z && action == GLFW_PRESS)
	{
		int linha = (int)iPos.x;
		int coluna = (int)iPos.y;
		if (tilemap[coluna-1][linha] == 3)
		{
			iPos.x = iPos.x;
			iPos.y = iPos.y;
		}
		else
		{
			dir = LEFT;
			player.moverParaEsquerda();
			iPos.y -= 1;
			if (linha == 6 && coluna == 1)
			{
				moedac1 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}
			if (linha == 11 && coluna == 10)
			{
				moedac2 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}
			if (linha == 10 && coluna == 11)
			{
				moedac3 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}
			if (linha == 11 && coluna == 11)
			{
				moedac4 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}
			if (linha == 10 && coluna == 10)
			{
				moedac5 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}	
		}
		
	}

	// DIREITA cima
	if (key == GLFW_KEY_E && action == GLFW_PRESS)
	{
		int linha = (int)iPos.x;
		int coluna = (int)iPos.y;
		if (tilemap[coluna+1][linha] == 3)
		{
			iPos.x = iPos.x;
			iPos.y = iPos.y;
		}
		else
		{
			player.moverParaDireita();
			dir = RIGHT;;
			iPos.y += 1;
			if (linha == 6 && coluna == 1)
			{
				moedac1 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}
			if (linha == 11 && coluna == 10)
			{
				moedac2 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}
			if (linha == 10 && coluna == 11)
			{
				moedac3 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}
			if (linha == 11 && coluna == 11)
			{
				moedac4 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}
			if (linha == 10 && coluna == 10)
			{
				moedac5 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}	
		}
		
	}

	// DIREITA reto
	if (key == GLFW_KEY_D && action == GLFW_PRESS)
	{
		int linha = (int)iPos.x;
		int coluna = (int)iPos.y;
		if (tilemap[coluna+1][linha+1] == 3)
		{
			iPos.x = iPos.x;
			iPos.y = iPos.y;
		}
		else
		{
			dir = RIGHT;
			player.moverParaDireita();
			iPos.x += 1;
			iPos.y += 1;
			if (linha == 6 && coluna == 1)
			{
				moedac1 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}
			if (linha == 11 && coluna == 10)
			{
				moedac2 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}
			if (linha == 10 && coluna == 11)
			{
				moedac3 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}
			if (linha == 11 && coluna == 11)
			{
				moedac4 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}
			if (linha == 10 && coluna == 10)
			{
				moedac5 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}	
		}
		
	}

	// DIREITA baixo
	if (key == GLFW_KEY_C && action == GLFW_PRESS)
	{
		int linha = (int)iPos.x;
		int coluna = (int)iPos.y;
		if (tilemap[coluna][linha+1] == 3)
		{
			iPos.x = iPos.x;
			iPos.y = iPos.y;
		}
		else
		{
			player.moverParaDireita();
			dir = RIGHT;
			iPos.x += 1;
			if (linha == 6 && coluna == 1)
			{
				moedac1 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}
			if (linha == 11 && coluna == 10)
			{
				moedac2 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}
			if (linha == 10 && coluna == 11)
			{
				moedac3 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}
			if (linha == 11 && coluna == 11)
			{
				moedac4 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}
			if (linha == 10 && coluna == 10)
			{
				moedac5 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}	
		}
	}
	
	// BAIXO reto
	if (key == GLFW_KEY_S && action == GLFW_PRESS)
	{
		int linha = (int)iPos.x;
		int coluna = (int)iPos.y;
		if (tilemap[coluna-1][linha+1] == 3)
		{
			iPos.x = iPos.x;
			iPos.y = iPos.y;
		}
		else
		{
			dir = UP;
			iPos.x += 1;
			iPos.y -= 1;
			player.moverParaBaixo();
			if (linha == 6 && coluna == 1)
			{
				moedac1 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}
			if (linha == 11 && coluna == 10)
			{
				moedac2 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}
			if (linha == 10 && coluna == 11)
			{
				moedac3 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}
			if (linha == 11 && coluna == 11)
			{
				moedac4 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}
			if (linha == 10 && coluna == 10)
			{
				moedac5 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}	
		}
	}

	// CIMA reto
	if (key == GLFW_KEY_W && action == GLFW_PRESS)
	{
		int linha = (int)iPos.x;
		int coluna = (int)iPos.y;
		if (tilemap[coluna+1][linha-1] == 3)
		{
			iPos.x = iPos.x;
			iPos.y = iPos.y;
		}
		else
		{
			dir = DOWN;
		iPos.x -= 1;
		iPos.y += 1;
		player.moverParaCima();
		if (linha == 6 && coluna == 1)
			{
				moedac1 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}
			if (linha == 11 && coluna == 10)
			{
				moedac2 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}
			if (linha == 10 && coluna == 11)
			{
				moedac3 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}
			if (linha == 11 && coluna == 11)
			{
				moedac4 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}
			if (linha == 10 && coluna == 10)
			{
				moedac5 = true;
				pontuacao += 1;
				cout << "Você coletou uma moeda! Você precisa coletar 5. Total: " << pontuacao << endl;
			}	
		}
	}
		
	if (action == GLFW_RELEASE)
	{
		dir = NONE;
		cout << "Posição atual: (" << iPos.y << ", " << iPos.x << ")" << endl;
		if (pontuacao >= 5)
		{
			cout << "Você ganhou o jogo! Coletou todas as moedas. Parabéns!" << endl;
		}
	}

}

GLuint loadTexture(string filePath, int &imgWidth, int &imgHeight)
{
	GLuint texID;

	// Gera o identificador da textura na memória 
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nrChannels;
	unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);

	if (data)
	{
    	if (nrChannels == 3) //jpg, bmp
    	{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    	}
    	else //png
    	{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    	}
    	glGenerateMipmap(GL_TEXTURE_2D);

		imgWidth = width;
		imgHeight = height;

		stbi_image_free(data);

		glBindTexture(GL_TEXTURE_2D, 0);

	}
	else
	{
    	 std::cout << "Failed to load texture" << std::endl;
	}
	return texID;
}

//Função para fazer a leitura do tilemap do arquivo
void loadMap(string fileName)
{
	ifstream arqEntrada;
	arqEntrada.open(fileName); //abertura do arquivo
	if (arqEntrada)
	{
		///leitura dos dados
		string textureName;
		int width, height;
		//Leitura das informações sobre o tileset
		arqEntrada >> textureName >> nTiles >> tileSize.y >> tileSize.x;
		tilesetTexID = loadTexture(textureName, width, height);
		//Só pra debug, printar os dados
		cout << textureName << " " << nTiles << " " << tileSize.y << " " << tileSize.x << endl;
		//Leitura das informações sobre o mapa (tilemap)
		arqEntrada >> tilemapSize.y >> tilemapSize.x; //nro de linhas e de colunas do mapa
		cout << tilemapSize.y << " " << tilemapSize.x << endl;
		for (int i = 0; i < tilemapSize.y; i++) //percorrendo as linhas do mapa
		{
			for (int j = 0; j < tilemapSize.x; j++) //percorrendo as colunas do mapa
			{
				arqEntrada >> tilemap[i][j];
				cout << tilemap[i][j] << " ";
			}
			cout << endl;
		}
	
	}
	else
	{
		cout << "Houve um problema na leitura de " << fileName << endl;
	}
}

GLuint setupTile()
{
	GLuint VAO;

	offsetTex.s = 1.0/ (float) nTiles;
	offsetTex.t = 1.0;
	glm::vec3 cor; //temporario, dá pra tirar 
	cor.r = 1.0;
	cor.g = 0.0;
	cor.b = 1.0;
    //Especificação da geometria da sprite (quadrado, 2 triangulos)
    GLfloat vertices[] = {
		//x   y    z    r      g      b      s    t
		-0.5, 0.5, 0.0, cor.r, cor.g, cor.b, 0.0, offsetTex.t, //v0
        -0.5,-0.5, 0.0, cor.r, cor.g, cor.b, 0.0, 0.0, //v1
         0.5, 0.5, 0.0, cor.r, cor.g, cor.b, offsetTex.s, offsetTex.t, //v2
        -0.5,-0.5, 0.0, cor.r, cor.g, cor.b, 0.0, 0.0, //v1
         0.5,-0.5, 0.0, cor.r, cor.g, cor.b, offsetTex.s, 0.0, //v3
         0.5, 0.5, 0.0, cor.r, cor.g, cor.b, offsetTex.s, offsetTex.t  //v2
	};


	GLuint VBO;
	//Gera��o do identificador do VBO
	glGenBuffers(1, &VBO);
	//Faz a conex�o (vincula) do buffer como um buffer de array
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//Envia os dados do array de floats para o buffer da OpenGl
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	//Atributo layout 0 - Posição - 3 valores dos 8 que descrevem o vértice
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//Atributo layout 1 - Cor - 3 valores dos 8 que descrevem o vértice
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3* sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	//Atributo layout 2 - Coordenada de textura - 2 valores dos 8 que descrevem o vértice
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6* sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0); 

	glBindVertexArray(0);
	return VAO;
}

void drawMap(Shader &shader)
{
    shader.Use();

    glBindTexture(GL_TEXTURE_2D, tilesetTexID); // Conectando com a textura
    glBindVertexArray(VAOTile); // Conectando ao buffer de geometria

    // Tamanho da janela
    int windowWidth = 1600;
    int windowHeight = 1200;

    // Calcular o tamanho total do mapa isométrico
    float totalMapWidth = tilemapSize.x * tileSize.x / 2;
    float totalMapHeight = tilemapSize.y * tileSize.y / 2;

    // Calcular a posição inicial para centralizar o mapa
    glm::vec2 posIni;
    posIni.x = 50;
    posIni.y = 300;

    for (int i = 0; i < tilemapSize.y; i++)
    {
        for (int j = 0; j < tilemapSize.x; j++)
        {
            // Calcula as coordenadas isométricas
			float x_iso = (j * tileSize.x / 2 + i * tileSize.x / 2);
			float y_iso = (j * tileSize.y / 2 - i * tileSize.y / 2);

            // Atualiza a matriz de transformação do tile i j
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(posIni.x + x_iso, posIni.y + y_iso, 0.0f));
            model = glm::scale(model, glm::vec3(tileSize.x, tileSize.y, 1.0f));
            shader.setMat4("model", glm::value_ptr(model));

            int indiceTile = tilemap[i][j];
            shader.setVec2("offsetTex", indiceTile * offsetTex.x, offsetTex.y);
            
            // Chamada de desenho para o tile
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
}

void desenharMoedas()
{
	float tw = tileSize.x;
	float th = tileSize.y;
	float coinX = posIni.x + (coinPos.x * tw / 2 + coinPos.y * tw / 2);
	float coinY = posIni.y + (coinPos.x * th / 2 - coinPos.y * th / 2);
	coin.setPosicao(glm::vec3(coinX, coinY, 0.0));
	if (!moedac1)
	{
		coin.desenhar();
	}

	float coin1X = posIni.x + (coin1Pos.x * tw / 2 + coin1Pos.y * tw / 2);
	float coin1Y = posIni.y + (coin1Pos.x * th / 2 - coin1Pos.y * th / 2);
	coin1.setPosicao(glm::vec3(coin1X, coin1Y, 0.0));
	if (!moedac2)
	{
		coin1.desenhar();
	}

	float coin2X = posIni.x + (coin2Pos.x * tw / 2 + coin2Pos.y * tw / 2);
	float coin2Y = posIni.y + (coin2Pos.x * th / 2 - coin2Pos.y * th / 2);
	coin2.setPosicao(glm::vec3(coin2X, coin2Y, 0.0));
	if (!moedac3)
	{
		coin2.desenhar();
	}

	float coin3X = posIni.x + (coin3Pos.x * tw / 2 + coin3Pos.y * tw / 2);
	float coin3Y = posIni.y + (coin3Pos.x * th / 2 - coin3Pos.y * th / 2);
	coin3.setPosicao(glm::vec3(coin3X, coin3Y, 0.0));
	if (!moedac4)
	{
		coin3.desenhar();
	}

	float coin4X = posIni.x + (coin4Pos.x * tw / 2 + coin4Pos.y * tw / 2);
	float coin4Y = posIni.y + (coin4Pos.x * th / 2 - coin4Pos.y * th / 2);
	coin4.setPosicao(glm::vec3(coin4X, coin4Y, 0.0));
	if (!moedac5)
	{
		coin4.desenhar();
	}
}
