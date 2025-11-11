#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "productos.h"
#include "usuarios.h"

#define RUTA_PRODUCTOS "producto.txt"
#define RUTA_VENTAS "ventas.txt"
#define RUTA_VENDEDORES "vendedores.txt"

typedef struct {
    char codigo[MAX_CODIGO];
    char nombre[MAX_NOMBRE];
    int cantidad;
    double costo;
    double precio;
} ItemVenta;

static void imprimir_linea_factura(ItemVenta *it) {
    double subtotal = it->cantidad * it->precio;
    printf("%-8s %-25s %5d  %8.2f  %8.2f\n",
           it->codigo, it->nombre, it->cantidad, it->precio, subtotal);
}

static int obtener_ultimo_num_factura(const char *ruta) {
    FILE *f = fopen(ruta, "r");
    if (!f) return 0; // no existe -> empezar en 1
    char linea[512];
    int ult = 0;
    while (fgets(linea, sizeof(linea), f)) {
        if (strlen(linea) < 3) continue;
        char *p = strtok(linea, ",");
        if (!p) continue;
        int num = atoi(trim(p));
        if (num > ult) ult = num;
    }
    fclose(f);
    return ult;
}

static void obtener_fecha_actual(char *out, size_t n) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    snprintf(out, n, "%04d-%02d-%02d", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday);
}

int main(void) {
    Inventario inv;
    ListaVendedores lv;
    if (cargar_inventario(RUTA_PRODUCTOS, &inv) != 0) {
        fprintf(stderr, "Error: no se pudo leer %s\n", RUTA_PRODUCTOS);
        // Crear inventario vacío para trabajar
        inv.items = NULL; inv.n = 0;
    }
    if (cargar_vendedores(RUTA_VENDEDORES, &lv) != 0) {
        fprintf(stderr, "Error: no se pudo leer %s\n", RUTA_VENDEDORES);
        // Si no hay archivo de vendedores no permitimos entrar.
        liberar_inventario(&inv);
        return 1;
    }

    // Autenticación
    char usuario[64], clave[64], nombre_vendedor[128];
    int intentos = 0;
    int ok = 0;
    while (intentos < 3 && !ok) {
        printf("Usuario: ");
        if (!fgets(usuario, sizeof(usuario), stdin)) break;
        usuario[strcspn(usuario, "\n")] = '\0';
        printf("Clave: ");
        if (!fgets(clave, sizeof(clave), stdin)) break;
        clave[strcspn(clave, "\n")] = '\0';
        if (autenticar(&lv, usuario, clave, nombre_vendedor)) {
            ok = 1;
            break;
        } else {
            intentos++;
            printf("Usuario/clave incorrectos (%d/3)\n", intentos);
        }
    }
    if (!ok) {
        printf("Máximo intentos alcanzado. Saliendo.\n");
        liberar_inventario(&inv);
        liberar_vendedores(&lv);
        return 0;
    }

    printf("Bienvenido, %s\n", nombre_vendedor);

    // Iniciar registro de venta
    ItemVenta *carrito = NULL;
    size_t ncarrito = 0;

    while (1) {
        char codigo[MAX_CODIGO];
        printf("Ingrese codigo de producto (o 'fin' para terminar): ");
        if (!fgets(codigo, sizeof(codigo), stdin)) break;
        codigo[strcspn(codigo, "\n")] = '\0';
        if (strcmp(codigo, "fin") == 0) break;
        Producto *p = buscar_producto_por_codigo(&inv, codigo);
        if (!p) {
            printf("Codigo no encontrado.\n");
            continue;
        }
        printf("Producto: %s | Disponible: %d | Precio: %.2f\n", p->nombre, p->cantidad, p->precio);
        char s_cant[32];
        printf("Cantidad a vender: ");
        if (!fgets(s_cant, sizeof(s_cant), stdin)) break;
        int cant = atoi(s_cant);
        if (cant <= 0) {
            printf("Cantidad invalida.\n");
            continue;
        }
        if (cant > p->cantidad) {
            printf("No hay suficiente inventario. Intentelo de nuevo.\n");
            continue;
        }
        // agregar al carrito (si ya existe, sumar)
        int found = 0;
        for (size_t i=0;i<ncarrito;i++) {
            if (strcmp(carrito[i].codigo, p->codigo) == 0) {
                carrito[i].cantidad += cant;
                found = 1;
                break;
            }
        }
        if (!found) {
            ItemVenta *tmp = realloc(carrito, (ncarrito + 1) * sizeof(ItemVenta));
            if (!tmp) {
                fprintf(stderr, "Fallo memoria\n");
                break;
            }
            carrito = tmp;
            strncpy(carrito[ncarrito].codigo, p->codigo, MAX_CODIGO-1);
            strncpy(carrito[ncarrito].nombre, p->nombre, MAX_NOMBRE-1);
            carrito[ncarrito].cantidad = cant;
            carrito[ncarrito].costo = p->costo;
            carrito[ncarrito].precio = p->precio;
            ncarrito++;
        }
        printf("Producto agregado al carrito.\n");
        // preguntar si agregar mas
        printf("¿Desea agregar otro producto? (s/n): ");
        char resp[8];
        if (!fgets(resp, sizeof(resp), stdin)) break;
        if (resp[0] == 'n' || resp[0] == 'N') break;
    }

    if (ncarrito == 0) {
        printf("No se registraron productos. Saliendo.\n");
        liberar_inventario(&inv);
        liberar_vendedores(&lv);
        free(carrito);
        return 0;
    }

    // Mostrar factura preliminar
    int ultimo = obtener_ultimo_num_factura(RUTA_VENTAS);
    int num_factura = ultimo + 1;
    char fecha[32];
    obtener_fecha_actual(fecha, sizeof(fecha));
    printf("\n================ FACTURA ================\n");
    printf("Fecha: %s   Num factura: %d\n", fecha, num_factura);
    printf("CODIGO   NOMBRE                      CANT   P.U.      SUBTOTAL\n");
    double total = 0.0;
    for (size_t i=0;i<ncarrito;i++) {
        imprimir_linea_factura(&carrito[i]);
        total += carrito[i].cantidad * carrito[i].precio;
    }
    printf("TOTAL: %.2f\n", total);
    printf("Confirma venta? (s/n): ");
    char conf[8];
    if (!fgets(conf, sizeof(conf), stdin)) conf[0]='n';
    if (!(conf[0]=='s' || conf[0]=='S')) {
        printf("Venta cancelada.\n");
        liberar_inventario(&inv);
        liberar_vendedores(&lv);
        free(carrito);
        return 0;
    }

    // Actualizar inventario y registrar cada linea en ventas.txt
    // Guardar inventario primero
    for (size_t i=0;i<ncarrito;i++) {
        Producto *pp = buscar_producto_por_codigo(&inv, carrito[i].codigo);
        if (pp) pp->cantidad -= carrito[i].cantidad;
    }
    if (guardar_inventario(RUTA_PRODUCTOS, &inv) != 0) {
        fprintf(stderr, "Error guardando inventario.\n");
    }

    FILE *fv = fopen(RUTA_VENTAS, "a");
    if (!fv) {
        fprintf(stderr, "Error: no se pudo abrir %s para append\n", RUTA_VENTAS);
    } else {
        for (size_t i=0;i<ncarrito;i++) {
            // Num_factura,Codigo_producto,Nombre_producto,cantidad_vendidad,costo,precio_venta,fecha_venta
            fprintf(fv, "%d,%s,%s,%d,%.2f,%.2f,%s\n",
                    num_factura,
                    carrito[i].codigo,
                    carrito[i].nombre,
                    carrito[i].cantidad,
                    carrito[i].costo,
                    carrito[i].precio,
                    fecha);
        }
        fclose(fv);
    }

    printf("Venta registrada correctamente (factura %d).\n", num_factura);

    // liberar
    liberar_inventario(&inv);
    liberar_vendedores(&lv);
    free(carrito);

    return 0;
}
