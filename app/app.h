

int App_algumaLataPresente (lt *lata);

void App_ativaIntPres (WORD canais_presenca);

void App_desativaIntPres (WORD canais_presenca);

void App_rstLatas (lt *lata);

void App_medirLatas (lt *lata, WORD medicoes[]);

void App_attLedLatas (lt *lata);

void App_enviaMed (lt *lata);

int _round (float i); //math.h ta dando problema 

int App_numDig (int a);

void App_configuraClock ();

int App_tempMedia (int vec[]);

int App_pegarTemp (int x);

int App_lerCanal (int canal);

void App_bitmap (char *mem, unsigned int id, int temp);

WORD App_paridade (unsigned int a, unsigned int b);

void App_configuraMSP ();

void App_configuraRadio ();

WORD App_pegarCanaisPresenca (lt *lata);