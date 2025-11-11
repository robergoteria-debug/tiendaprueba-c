#ifndef USUARIOS_H
#define USUARIOS_H

#define MAX_USER 64
#define MAX_PASS 64
#define MAX_NOM 128

typedef struct {
    char usuario[MAX_USER];
    char clave[MAX_PASS];
    char nombre[MAX_NOM];
} Vendedor;

typedef struct {
    Vendedor *items;
    size_t n;
} ListaVendedores;

int cargar_vendedores(const char *ruta, ListaVendedores *lv);
void liberar_vendedores(ListaVendedores *lv);
int autenticar(ListaVendedores *lv, const char *usuario, const char *clave, char *nombre_out);

#endif
