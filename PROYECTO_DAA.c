#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <limits.h>

#define MAX_PRODUCTOS 100
#define MAX_LOCALIDADES 20
#define MAX_CAMIONES 5
#define MAX_CLIENTES 10
#define MAX_CONEXIONES 30
#define INF INT_MAX

typedef struct
{
    int id;
    char nombre[50];
    float precio;
    float peso;
    float volumen;
} Producto;

Producto productosArreglo[MAX_PRODUCTOS];

typedef struct
{
    Producto *producto;
    int cantidad;
} Pedido;

typedef struct
{
    Producto *producto;
    int cantidad;
    float valor_total;
    float peso_total;
    float volumen_total;
} ProductoSeleccionado;

typedef struct
{
    int id;
    char nombre[50];
    float latitud;
    float longitud;
    Pedido pedidos[MAX_PRODUCTOS];
    int total_pedidos;
} Localidad;

Localidad localidadesArreglo[MAX_LOCALIDADES];

typedef enum
{
    DISPONIBLE,
    EN_RUTA,
    MANTENIMIENTO
} EstadoCamion;

typedef struct
{
    int id;
    float capacidadCarga;   // kg
    float capacidadVolumen; // m³
    EstadoCamion estado;
    int ruta[MAX_LOCALIDADES]; // ids de localidades
    int total_ruta;
    int origen_id;
    int destino_id;
} Camion;

Camion camionesArreglo[MAX_CAMIONES];

typedef struct
{
    int id;
    char nombre[50];
    char direccion[100];
    Pedido historial[100];
    int total_pedidos;
} Cliente;

Cliente clientesArreglo[MAX_CLIENTES];

typedef struct
{
    int origen_id;
    int destino_id;
    float distancia_km;
    float tiempo_min;
    float penalizacion_trafico; // en minutos
} Conexion;

Conexion conexionesArreglo[MAX_CONEXIONES];

void precargarDatos()
{
    srand(time(NULL));

    // PRECARGA DE PRODUCTOS
    for (int i = 0; i < MAX_PRODUCTOS; i++)
    {
        productosArreglo[i].id = i + 1;
        sprintf(productosArreglo[i].nombre, "Producto %d", (i + 1));
        productosArreglo[i].precio = (rand() % 200);
        productosArreglo[i].peso = (rand() % 50);
        productosArreglo[i].volumen = (rand() % 50);
    }

    // PRECARGA DE LOCALIDADES
    for (int i = 0; i < MAX_LOCALIDADES; i++)
    {
        localidadesArreglo[i].id = i + 1;
        sprintf(localidadesArreglo[i].nombre, "Localidad %d", (i + 1));
        localidadesArreglo[i].latitud = (float)(rand() % 90);
        localidadesArreglo[i].longitud = (float)(rand() % 180);
        localidadesArreglo[i].total_pedidos = 1 + rand() % 6; // 1 a 6 pedidos

        for (int j = 0; j < localidadesArreglo[i].total_pedidos; j++)
        {
            int productoIndex = rand() % MAX_PRODUCTOS;
            localidadesArreglo[i].pedidos[j].producto = &productosArreglo[productoIndex];
            localidadesArreglo[i].pedidos[j].cantidad = 1 + rand() % 10;
        }
    }

    // PRECARGA DE CAMIONES
    for (int i = 0; i < MAX_CAMIONES; i++)
    {
        camionesArreglo[i].id = i + 1;
        camionesArreglo[i].capacidadCarga = 500 + (rand() % 1000); // kg
        camionesArreglo[i].capacidadVolumen = 10 + (rand() % 50);  // m3
        camionesArreglo[i].estado = DISPONIBLE;
        camionesArreglo[i].total_ruta = 0;

        do
        {
            camionesArreglo[i].origen_id = 1 + rand() % MAX_LOCALIDADES;
            camionesArreglo[i].destino_id = 1 + rand() % MAX_LOCALIDADES;
        } while (camionesArreglo[i].origen_id == camionesArreglo[i].destino_id);
    }

    // PRECARGA DE CLIENTES
    for (int i = 0; i < MAX_CLIENTES; i++)
    {
        clientesArreglo[i].id = i + 1;
        sprintf(clientesArreglo[i].nombre, "Cliente %d", i + 1);
        sprintf(clientesArreglo[i].direccion, "Calle Ficticia #%d", 100 + i);
        clientesArreglo[i].total_pedidos = rand() % 10 + 1;
        for (int j = 0; j < clientesArreglo[i].total_pedidos; j++)
        {
            int productoIndex = rand() % MAX_PRODUCTOS;
            clientesArreglo[i].historial[j].producto = &productosArreglo[productoIndex];
            clientesArreglo[i].historial[j].cantidad = rand() % 5 + 1;
        }
    }

    // PRECARGA DE CONEXIONES ENTRE LOCALIDADES
    int total_conexiones = 0;

    // PASO 1: Conexiones mínimas para asegurar conectividad
    for (int i = 0; i < MAX_LOCALIDADES - 1; i++)
    {
        conexionesArreglo[total_conexiones].origen_id = i + 1;
        conexionesArreglo[total_conexiones].destino_id = i + 2;
        conexionesArreglo[total_conexiones].distancia_km = 5 + (rand() % 100);
        conexionesArreglo[total_conexiones].tiempo_min = 10 + (rand() % 120);
        conexionesArreglo[total_conexiones].penalizacion_trafico = (rand() % 3) * 5;
        total_conexiones++;
    }

    // PASO 2: Conexiones aleatorias adicionales
    while (total_conexiones < MAX_CONEXIONES)
    {
        int origen = rand() % MAX_LOCALIDADES + 1;
        int destino = rand() % MAX_LOCALIDADES + 1;

        if (origen == destino)
            continue;

        // Validar que no exista ya esa conexión
        bool existe = false;
        for (int j = 0; j < total_conexiones; j++)
        {
            if ((conexionesArreglo[j].origen_id == origen && conexionesArreglo[j].destino_id == destino) ||
                (conexionesArreglo[j].origen_id == destino && conexionesArreglo[j].destino_id == origen))
            {
                existe = true;
                break;
            }
        }

        if (existe)
            continue;

        conexionesArreglo[total_conexiones].origen_id = origen;
        conexionesArreglo[total_conexiones].destino_id = destino;
        conexionesArreglo[total_conexiones].distancia_km = 5 + (rand() % 100);
        conexionesArreglo[total_conexiones].tiempo_min = 10 + (rand() % 120);
        conexionesArreglo[total_conexiones].penalizacion_trafico = (rand() % 3) * 5;
        total_conexiones++;
    }
}

void imprimir_resultados_productos(int id_camion, const char* nombre_localidad, ProductoSeleccionado* seleccionados, int totalSeleccionados) {
    printf("\nCamion #%d -> Localidad \"%s\":\n", id_camion, nombre_localidad);

    if (totalSeleccionados == 0) {
        printf("  No se asignaron productos.\n");
    } else {
        float acumulado = 0;
        for (int k = 0; k < totalSeleccionados; k++) {
            acumulado += seleccionados[k].valor_total;
            printf("  - %s | Cantidad: %d | Valor: %.2f | Peso: %.2f | Volumen: %.2f\n",
                   seleccionados[k].producto->nombre,
                   seleccionados[k].cantidad,
                   seleccionados[k].valor_total,
                   seleccionados[k].peso_total,
                   seleccionados[k].volumen_total);
        }
        printf("  TOTAL VALOR ACUMULADO: %.2f\n", acumulado);
    }
}


void liberar_tabla_memoizacion(float*** tabla, int n, int pesoMax, int volumenMax) {
    for (int i = 0; i < n; i++) {
        for (int w = 0; w <= pesoMax; w++) {
            free(tabla[i][w]);
        }
        free(tabla[i]);
    }
    free(tabla);
}


// Memoización: dp[i][w][v] == -1.0 si no está calculado
float resolver_top_down(Pedido *pedidos, int i, int peso_restante, int volumen_restante, float ***tabla)
{
    if (i < 0 || peso_restante < 0 || volumen_restante < 0)
        return 0;

    if (tabla[i][peso_restante][volumen_restante] != -1)
        return tabla[i][peso_restante][volumen_restante];

    Pedido actual = pedidos[i];
    int cantidad = actual.cantidad;
    int peso = (int)(actual.producto->peso * cantidad);
    int volumen = (int)(actual.producto->volumen * cantidad);
    float valor = actual.producto->precio * cantidad;

    float sin_tomar = resolver_top_down(pedidos, i - 1, peso_restante, volumen_restante, tabla);

    float tomar = 0;
    if (peso <= peso_restante && volumen <= volumen_restante)
        tomar = valor + resolver_top_down(pedidos, i - 1, peso_restante - peso, volumen_restante - volumen, tabla);

    tabla[i][peso_restante][volumen_restante] = fmax(sin_tomar, tomar);
    return tabla[i][peso_restante][volumen_restante];
}

void asignacion_optima_productos_camiones()
{
    for (int i = 0; i < MAX_CAMIONES; i++)
    {
        Camion *camion = &camionesArreglo[i];
        if (camion->estado != DISPONIBLE)
            continue;

        for (int j = 0; j < MAX_LOCALIDADES; j++)
        {
            Localidad *localidad = &localidadesArreglo[j];
            if (localidad->total_pedidos == 0)
                continue;

            int n = localidad->total_pedidos;
            int pesoMax = (int)camion->capacidadCarga;
            int volumenMax = (int)camion->capacidadVolumen;

            float ***tabla = malloc(n * sizeof(float **));
            for (int i = 0; i < n; i++)
            {
                tabla[i] = malloc((pesoMax + 1) * sizeof(float *));
                for (int w = 0; w <= pesoMax; w++)
                {
                    tabla[i][w] = malloc((volumenMax + 1) * sizeof(float));
                    for (int v = 0; v <= volumenMax; v++)
                        tabla[i][w][v] = -1.0f;
                }
            }

            resolver_top_down(localidad->pedidos, n - 1, pesoMax, volumenMax, tabla);

            ProductoSeleccionado seleccionados[MAX_PRODUCTOS];
            int totalSeleccionados = 0;
            int w = pesoMax, v = volumenMax;

            for (int k = n - 1; k >= 0; k--)
            {
                int cantidad = localidad->pedidos[k].cantidad;
                int peso = (int)(localidad->pedidos[k].producto->peso * cantidad);
                int volumen = (int)(localidad->pedidos[k].producto->volumen * cantidad);
                float valor = localidad->pedidos[k].producto->precio * cantidad;

                float sin_tomar = (k == 0) ? 0 : tabla[k - 1][w][v];
                float actual = tabla[k][w][v];

                if (peso <= w && volumen <= v && actual > sin_tomar)
                {
                    seleccionados[totalSeleccionados].producto = localidad->pedidos[k].producto;
                    seleccionados[totalSeleccionados].cantidad = cantidad;
                    seleccionados[totalSeleccionados].valor_total = valor;
                    seleccionados[totalSeleccionados].peso_total = peso;
                    seleccionados[totalSeleccionados].volumen_total = volumen;
                    totalSeleccionados++;

                    w -= peso;
                    v -= volumen;
                }
            }

            // Imprimir e imprimir
            imprimir_resultados_productos(camion->id, localidad->nombre, seleccionados, totalSeleccionados);
            liberar_tabla_memoizacion(tabla, n, pesoMax, volumenMax);
        }
    }
}


float matriz_tiempo[MAX_LOCALIDADES][MAX_LOCALIDADES];
float matriz_distancia[MAX_LOCALIDADES][MAX_LOCALIDADES];

void inicializar_matrices_rutas()
{
    for (int i = 0; i < MAX_LOCALIDADES; i++)
    {
        for (int j = 0; j < MAX_LOCALIDADES; j++)
        {
            matriz_tiempo[i][j] = (i == j) ? 0 : INF;
            matriz_distancia[i][j] = (i == j) ? 0 : INF;
        }
    }

    for (int i = 0; i < MAX_CONEXIONES; i++)
    {
        int u = conexionesArreglo[i].origen_id - 1;
        int v = conexionesArreglo[i].destino_id - 1;
        float tiempo = conexionesArreglo[i].tiempo_min + conexionesArreglo[i].penalizacion_trafico;
        float distancia = conexionesArreglo[i].distancia_km;

        if (tiempo < matriz_tiempo[u][v])
            matriz_tiempo[u][v] = tiempo;
        if (distancia < matriz_distancia[u][v])
            matriz_distancia[u][v] = distancia;
    }
}

void dijkstra_tiempo(int origen, float resultado[MAX_LOCALIDADES])
{
    bool visitado[MAX_LOCALIDADES] = {false};

    for (int i = 0; i < MAX_LOCALIDADES; i++)
        resultado[i] = INF;
    resultado[origen] = 0;

    for (int i = 0; i < MAX_LOCALIDADES - 1; i++)
    {
        int u = -1;
        float min_dist = INF;
        for (int j = 0; j < MAX_LOCALIDADES; j++)
        {
            if (!visitado[j] && resultado[j] < min_dist)
            {
                u = j;
                min_dist = resultado[j];
            }
        }

        if (u == -1)
            break;
        visitado[u] = true;

        for (int v = 0; v < MAX_LOCALIDADES; v++)
        {
            if (!visitado[v] && matriz_tiempo[u][v] < INF &&
                resultado[u] + matriz_tiempo[u][v] < resultado[v])
            {
                resultado[v] = resultado[u] + matriz_tiempo[u][v];
            }
        }
    }
}

void floyd_distancia(float resultado[MAX_LOCALIDADES][MAX_LOCALIDADES])
{
    for (int i = 0; i < MAX_LOCALIDADES; i++)
        for (int j = 0; j < MAX_LOCALIDADES; j++)
            resultado[i][j] = matriz_distancia[i][j];

    for (int k = 0; k < MAX_LOCALIDADES; k++)
        for (int i = 0; i < MAX_LOCALIDADES; i++)
            for (int j = 0; j < MAX_LOCALIDADES; j++)
                if (resultado[i][k] + resultado[k][j] < resultado[i][j])
                    resultado[i][j] = resultado[i][k] + resultado[k][j];
}

void optimizar_y_asignar_rutas(int modo)
{
    inicializar_matrices_rutas();

    printf("\n==================================================\n");
    if (modo == 1)
    {
        printf("   RUTAS OPTIMIZADAS POR TIEMPO \n");
        printf("==================================================\n");
        printf("| Camion | Origen        -> Destino       | Tiempo Estimado |\n");
        printf("--------------------------------------------------------------\n");

        for (int i = 0; i < MAX_CAMIONES; i++)
        {
            if (camionesArreglo[i].estado != DISPONIBLE)
                continue;

            int origen = camionesArreglo[i].origen_id - 1;
            int destino = camionesArreglo[i].destino_id - 1;

            float distancias[MAX_LOCALIDADES];
            dijkstra_tiempo(origen, distancias);

            float tiempo_total = distancias[destino];
            camionesArreglo[i].ruta[0] = camionesArreglo[i].destino_id;
            camionesArreglo[i].total_ruta = 1;

            printf("|   #%2d   | %-13s -> %-13s |   %6.1f min     |\n",
                   camionesArreglo[i].id,
                   localidadesArreglo[origen].nombre,
                   localidadesArreglo[destino].nombre,
                   tiempo_total);
        }
    }
    else if (modo == 2)
    {
        printf("   RUTAS OPTIMIZADAS POR DISTANCIA \n");
        printf("=====================================================\n");
        printf("| Camion | Origen        -> Destino       | Distancia Total |\n");
        printf("--------------------------------------------------------------\n");

        float resultado[MAX_LOCALIDADES][MAX_LOCALIDADES];
        floyd_distancia(resultado);

        for (int i = 0; i < MAX_CAMIONES; i++)
        {
            if (camionesArreglo[i].estado != DISPONIBLE)
                continue;

            int origen = camionesArreglo[i].origen_id - 1;
            int destino = camionesArreglo[i].destino_id - 1;

            float distancia_total = resultado[origen][destino];
            camionesArreglo[i].ruta[0] = camionesArreglo[i].destino_id;
            camionesArreglo[i].total_ruta = 1;

            printf("|   #%2d   | %-13s -> %-13s |   %6.1f km      |\n",
                   camionesArreglo[i].id,
                   localidadesArreglo[origen].nombre,
                   localidadesArreglo[destino].nombre,
                   distancia_total);
        }
    }
    else
    {
        printf("Modo invalido. Usa 1 para TIEMPO o 2 para DISTANCIA.\n");
    }
    printf("--------------------------------------------------------------\n");
}

int main()
{
    int opcion;
    precargarDatos();

    do
    {
        printf("\n\n===== UVS EXPRESS - MENU PRINCIPAL =====\n");
        printf("1. Asignar productos optimamente a camiones\n");
        printf("2. Optimizar rutas y asignar a camiones (modo tiempo)\n");
        printf("3. Optimizar rutas y asignar a camiones (modo distancia)\n");
        printf("0. Salir\n");
        printf("Selecciona una opcion: ");
        scanf("%d", &opcion);

        switch (opcion)
        {
        case 1:
            asignacion_optima_productos_camiones();
            break;
        case 2:
            optimizar_y_asignar_rutas(1);
            break;
        case 3:
            optimizar_y_asignar_rutas(2);
            break;
        case 0:
            printf("Saliendo del programa...\n");
            break;
        default:
            printf("Opcion invalida. Intenta de nuevo.\n");
        }
    } while (opcion != 0);

    return 0;
}
