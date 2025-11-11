#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "productos.h"

char *trim(char *s) {
    if (!s) return s;
    // trim leading
    while(*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') s++;
    // trim trailing
    char *end = s + strlen(s) - 1;
    while(end > s && (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n')) end--;
    *(end+1) = '\0';
    return s;
}

int cargar_inventario(const char *ruta, Inventario *inv) {
    FILE *f = fopen(ruta, "r");
    if (!f) return -1;
    inv->items = NULL;
    inv->n = 0;
    char linea[512];
    while (fgets(linea, sizeof(linea), f)) {
        if (strlen(linea) < 3) continue;
        char *p = linea;
        char *token;
        token = strtok(p, ",");
        if (!token) continue;
        Producto prod;
        strncpy(prod.codigo, trim(token), MAX_CODIGO-1);
        prod.codigo[MAX_CODIGO-1]='\0';

        token = strtok(NULL, ",");
        if (!token) continue;
        strncpy(prod.nombre, trim(token), MAX_NOMBRE-1);
        prod.nombre[MAX_NOMBRE-1]='\0';

        token = strtok(NULL, ",");
        if (!token) continue;
        prod.cantidad = atoi(trim(token));

        token = strtok(NULL, ",");
        if (!token) continue;
        prod.costo = atof(trim(token));

        token = strtok(NULL, ",");
        if (!token) continue;
        prod.precio = atof(trim(token));

        // append
        Producto *tmp = realloc(inv->items, (inv->n + 1) * sizeof(Producto));
        if (!tmp) { fclose(f); return -2; }
        inv->items = tmp;
        inv->items[inv->n++] = prod;
    }
    fclose(f);
    return 0;
}

int guardar_inventario(const char *ruta, Inventario *inv) {
    FILE *f = fopen(ruta, "w");
    if (!f) return -1;
    for (size_t i=0;i<inv->n;i++) {
        Producto *p = &inv->items[i];
        fprintf(f, "%s,%s,%d,%.2f,%.2f\n",
                p->codigo, p->nombre, p->cantidad, p->costo, p->precio);
    }
    fclose(f);
    return 0;
}

Producto* buscar_producto_por_codigo(Inventario *inv, const char *codigo) {
    if (!inv) return NULL;
    for (size_t i=0;i<inv->n;i++) {
        if (strcmp(inv->items[i].codigo, codigo) == 0) return &inv->items[i];
    }
    return NULL;
}

void liberar_inventario(Inventario *inv) {
    if (!inv) return;
    free(inv->items);
    inv->items = NULL;
    inv->n = 0;
}
