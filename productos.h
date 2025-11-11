#ifndef PRODUCTOS_H
#define PRODUCTOS_H

#define MAX_CODIGO 64
#define MAX_NOMBRE 128

typedef struct {
    char codigo[MAX_CODIGO];
    char nombre[MAX_NOMBRE];
    int cantidad;
    double costo;
    double precio;
} Producto;

typedef struct {
    Producto *items;
    size_t n;
} Inventario;

typedef struct {
    char codigo[MAX_CODIGO];
    char nombre[MAX_NOMBRE];
    int cantidad_vendida;
    double costo;
    double precio;
} VentaLinea;

/* inventario */
int cargar_inventario(const char *ruta, Inventario *inv);
int guardar_inventario(const char *ruta, Inventario *inv);
Producto* buscar_producto_por_codigo(Inventario *inv, const char *codigo);
void liberar_inventario(Inventario *inv);

/* util */
char *trim(char *s);

#endif
