#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "usuarios.h"

/* trim helper (simple) */
static char *trim_local(char *s) {
    if (!s) return s;
    while(*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') s++;
    char *end = s + strlen(s) - 1;
    while(end > s && (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n')) end--;
    *(end+1) = '\0';
    return s;
}

int cargar_vendedores(const char *ruta, ListaVendedores *lv) {
    FILE *f = fopen(ruta, "r");
    if (!f) return -1;
    lv->items = NULL;
    lv->n = 0;
    char linea[512];
    while (fgets(linea, sizeof(linea), f)) {
        if (strlen(linea) < 3) continue;
        char *token = strtok(linea, ",");
        if (!token) continue;
        Vendedor v;
        strncpy(v.usuario, trim_local(token), MAX_USER-1);
        v.usuario[MAX_USER-1] = '\\0';
        token = strtok(NULL, ",");
        if (!token) continue;
        strncpy(v.clave, trim_local(token), MAX_PASS-1);
        v.clave[MAX_PASS-1] = '\\0';
        token = strtok(NULL, ",");
        if (!token) { v.nombre[0]='\\0'; }
        else {
            strncpy(v.nombre, trim_local(token), MAX_NOM-1);
            v.nombre[MAX_NOM-1] = '\\0';
        }
        Vendedor *tmp = realloc(lv->items, (lv->n + 1) * sizeof(Vendedor));
        if (!tmp) { fclose(f); return -2; }
        lv->items = tmp;
        lv->items[lv->n++] = v;
    }
    fclose(f);
    return 0;
}

void liberar_vendedores(ListaVendedores *lv) {
    if (!lv) return;
    free(lv->items);
    lv->items = NULL;
    lv->n = 0;
}

int autenticar(ListaVendedores *lv, const char *usuario, const char *clave, char *nombre_out) {
    if (!lv) return 0;
    for (size_t i=0;i<lv->n;i++) {
        if (strcmp(lv->items[i].usuario, usuario) == 0 &&
            strcmp(lv->items[i].clave, clave) == 0) {
            if (nombre_out) strncpy(nombre_out, lv->items[i].nombre, MAX_NOM-1);
            return 1;
        }
    }
    return 0;
}
