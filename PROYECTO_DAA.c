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
#define HASH_SIZE 128

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

int total_no = 0;
int total_si = 0;
ProductoSeleccionado si_seleccionados[MAX_PRODUCTOS];
ProductoSeleccionado no_seleccionados[MAX_PRODUCTOS];

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
    int nodo;
    float distancia;
} NodoHeap;

typedef struct
{
    int id;
    float capacidadCarga;   // kg
    float capacidadVolumen; // m cubicos
    EstadoCamion estado;
    int ruta[MAX_LOCALIDADES]; // id de localidades
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

typedef struct NodoHash
{
    ProductoSeleccionado prod;
    struct NodoHash *sig;
} NodoHash;

NodoHash *tablaHash[HASH_SIZE];

void precargarDatos()
{
    srand(time(NULL));

    // PRECARGA DE PRODUCTOS
    for (int i = 0; i < MAX_PRODUCTOS; i++)
    {
        productosArreglo[i].id = i + 1;
        sprintf(productosArreglo[i].nombre, "Producto %d", (i + 1));
        productosArreglo[i].precio = 10 + (rand() % 200);
        productosArreglo[i].peso = 2 + (rand() % 50);
        productosArreglo[i].volumen = 1 + (rand() % 50);
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

void imprimir_resultados_productos(int id_camion, const char *nombre_localidad, ProductoSeleccionado *seleccionados, int totalSeleccionados, int deci)
{
    printf("\nCamion #%d que va a la Localidad \"%s\":\n", id_camion, nombre_localidad);

    if (deci == 0)
    {
        // printf("  No se asignaron productos.\n");
        no_seleccionados[total_no] = seleccionados[0];
        total_no++;
        printf("  TOTAL DE PRODUCTOS NO ASIGNADOS: %d\n", total_no);
        printf("  Producto no asignado: %s | Cantidad: %d | Peso: %.2f | Volumen: %.2f\n",
               seleccionados[0].producto->nombre,
               seleccionados[0].cantidad,
               seleccionados[0].peso_total,
               seleccionados[0].volumen_total);
    }
    else
    {

        float acumulado = 0;
        for (int k = 0; k < totalSeleccionados; k++)
        {
            si_seleccionados[total_si + k] = seleccionados[k];

            acumulado += seleccionados[k].valor_total;
            printf("  - %s  | Valor: %.2f | Peso: %.2f | Volumen: %.2f\n",
                   seleccionados[k].producto->nombre,
                   seleccionados[k].valor_total,
                   seleccionados[k].peso_total,
                   seleccionados[k].volumen_total);
        }
        printf("  VALOR TOTAL ACUMULADO DE: %.2f\n", acumulado);
        total_si += totalSeleccionados;
        printf("  TOTAL DE PRODUCTOS ASIGNADOS: %d\n", total_si);
    }
}

void liberar_tabla_memoizacion(float ***tabla, int n, int pesoMax)
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j <= pesoMax; j++)
        {
            free(tabla[i][j]);
        }
        free(tabla[i]);
    }
    free(tabla);
}

typedef struct
{
    bool *decisiones;
} DecisionProductos;

float resolver_top_down(Pedido *pedidos, int i, int peso_restante, int volumen_restante,
                        float ***tabla, DecisionProductos *dp)
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

    float sin_tomar = resolver_top_down(pedidos, i - 1, peso_restante, volumen_restante, tabla, dp);

    float tomar = 0;
    if (peso <= peso_restante && volumen <= volumen_restante)
    {
        tomar = valor + resolver_top_down(pedidos, i - 1, peso_restante - peso,
                                          volumen_restante - volumen, tabla, dp);
    }

    // Guardar decisión
    if (tomar > sin_tomar)
    {
        dp->decisiones[i] = true;
        tabla[i][peso_restante][volumen_restante] = tomar;
    }
    else
    {
        dp->decisiones[i] = false;
        tabla[i][peso_restante][volumen_restante] = sin_tomar;
    }

    return tabla[i][peso_restante][volumen_restante];
}

int no_totalSeleccionados = 0;
ProductoSeleccionado seleccionados_no[MAX_PRODUCTOS];
ProductoSeleccionado seleccionados[MAX_PRODUCTOS];
int totalSeleccionados = 0;

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

            // Inicializar tabla
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

            // Inicializar estructura de decisiones
            DecisionProductos dp;
            dp.decisiones = calloc(n, sizeof(bool));

            // Resolver problema
            resolver_top_down(localidad->pedidos, n - 1, pesoMax, volumenMax, tabla, &dp);

            // Reiniciar contadores
            totalSeleccionados = 0;
            no_totalSeleccionados = 0;
            int deci = 0;
            // Reconstruir solución
            for (int k = 0; k < n; k++)
            {
                Pedido actual = localidad->pedidos[k];
                int cantidad = actual.cantidad;
                float valor = actual.producto->precio * cantidad;
                int peso = (int)(actual.producto->peso * cantidad);
                int volumen = (int)(actual.producto->volumen * cantidad);

                if (dp.decisiones[k])
                {
                    // Agregar a seleccionados
                    seleccionados[totalSeleccionados].producto = actual.producto;
                    seleccionados[totalSeleccionados].cantidad = cantidad;
                    seleccionados[totalSeleccionados].valor_total = valor;
                    seleccionados[totalSeleccionados].peso_total = peso;
                    seleccionados[totalSeleccionados].volumen_total = volumen;
                    totalSeleccionados++;
                    deci = 1;
                }
                else
                {
                    // Agregar a no seleccionados
                    seleccionados_no[no_totalSeleccionados].producto = actual.producto;
                    seleccionados_no[no_totalSeleccionados].cantidad = cantidad;
                    seleccionados_no[no_totalSeleccionados].valor_total = valor;
                    seleccionados_no[no_totalSeleccionados].peso_total = peso;
                    seleccionados_no[no_totalSeleccionados].volumen_total = volumen;
                    no_totalSeleccionados++;
                    deci = 0;
                }
            }

            // Imprimir resultados y liberar memoria
            imprimir_resultados_productos(camion->id, localidad->nombre,
                                          seleccionados, totalSeleccionados, deci);
            liberar_tabla_memoizacion(tabla, n, pesoMax);
            free(dp.decisiones);
        }
    }

    /*for (int i = 0; i < total_no; i++)
    {
        printf("\n==================================================\n");
        printf("Producto no asignado: %s | Cantidad: %d | Peso: %.2f | Volumen: %.2f\n",
               no_seleccionados[i].producto->nombre,
               no_seleccionados[i].cantidad,
               no_seleccionados[i].peso_total,
               no_seleccionados[i].volumen_total);
    }*/
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

void intercambiar(NodoHeap *a, NodoHeap *b)
{
    NodoHeap temp = *a;
    *a = *b;
    *b = temp;
}

void heapify_arriba(NodoHeap heap[], int idx)
{
    while (idx > 0)
    {
        int padre = (idx - 1) / 2;
        if (heap[idx].distancia < heap[padre].distancia)
        {
            intercambiar(&heap[idx], &heap[padre]);
            idx = padre;
        }
        else
            break;
    }
}

void heapify_abajo(NodoHeap heap[], int size, int idx)
{
    while (1)
    {
        int menor = idx;
        int izq = 2 * idx + 1;
        int der = 2 * idx + 2;

        if (izq < size && heap[izq].distancia < heap[menor].distancia)
            menor = izq;
        if (der < size && heap[der].distancia < heap[menor].distancia)
            menor = der;

        if (menor != idx)
        {
            intercambiar(&heap[idx], &heap[menor]);
            idx = menor;
        }
        else
            break;
    }
}

void insertar_heap(NodoHeap heap[], int *size, int nodo, float dist)
{
    heap[*size].nodo = nodo;
    heap[*size].distancia = dist;
    heapify_arriba(heap, *size);
    (*size)++;
}

NodoHeap extraer_min(NodoHeap heap[], int *size)
{
    NodoHeap min = heap[0];
    heap[0] = heap[--(*size)];
    heapify_abajo(heap, *size, 0);
    return min;
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
            bool visitado[MAX_LOCALIDADES] = {false};
            NodoHeap heap[MAX_LOCALIDADES * MAX_LOCALIDADES];
            int heap_size = 0;

            for (int k = 0; k < MAX_LOCALIDADES; k++)
                distancias[k] = INF;
            distancias[origen] = 0;
            insertar_heap(heap, &heap_size, origen, 0);

            while (heap_size > 0)
            {
                NodoHeap actual = extraer_min(heap, &heap_size);
                int u = actual.nodo;

                if (visitado[u])
                    continue;
                visitado[u] = true;

                for (int v = 0; v < MAX_LOCALIDADES; v++)
                {
                    float peso = matriz_tiempo[u][v];
                    if (peso < INF && distancias[u] + peso < distancias[v])
                    {
                        distancias[v] = distancias[u] + peso;
                        insertar_heap(heap, &heap_size, v, distancias[v]);
                    }
                }
            }

            float tiempo_total = distancias[destino];
            camionesArreglo[i].ruta[0] = camionesArreglo[i].destino_id;
            camionesArreglo[i].total_ruta = 1;

            if (tiempo_total >= INF)
            {
                printf("|   #%2d   | %-13s -> %-13s |   RUTA NO DISP.  |\n",
                       camionesArreglo[i].id,
                       localidadesArreglo[origen].nombre,
                       localidadesArreglo[destino].nombre);
            }
            else
            {
                printf("|   #%2d   | %-13s -> %-13s |   %6.1f min     |\n",
                       camionesArreglo[i].id,
                       localidadesArreglo[origen].nombre,
                       localidadesArreglo[destino].nombre,
                       tiempo_total);
            }
        }
    }
    else if (modo == 2)
    {
        printf("   RUTAS OPTIMIZADAS POR DISTANCIA \n");
        printf("=====================================================\n");
        printf("| Camion | Origen        -> Destino       | Distancia Total |\n");
        printf("--------------------------------------------------------------\n");

        float resultado[MAX_LOCALIDADES][MAX_LOCALIDADES];

        for (int i = 0; i < MAX_LOCALIDADES; i++)
            for (int j = 0; j < MAX_LOCALIDADES; j++)
                resultado[i][j] = matriz_distancia[i][j];

        for (int k = 0; k < MAX_LOCALIDADES; k++)
            for (int i = 0; i < MAX_LOCALIDADES; i++)
                for (int j = 0; j < MAX_LOCALIDADES; j++)
                    if (resultado[i][k] + resultado[k][j] < resultado[i][j])
                        resultado[i][j] = resultado[i][k] + resultado[k][j];

        for (int i = 0; i < MAX_CAMIONES; i++)
        {
            if (camionesArreglo[i].estado != DISPONIBLE)
                continue;

            int origen = camionesArreglo[i].origen_id - 1;
            int destino = camionesArreglo[i].destino_id - 1;

            float distancia_total = resultado[origen][destino];
            camionesArreglo[i].ruta[0] = camionesArreglo[i].destino_id;
            camionesArreglo[i].total_ruta = 1;

            if (distancia_total >= INF)
            {
                printf("|   #%2d   | %-13s -> %-13s |   RUTA NO DISP.  |\n",
                       camionesArreglo[i].id,
                       localidadesArreglo[origen].nombre,
                       localidadesArreglo[destino].nombre);
            }
            else
            {
                printf("|   #%2d   | %-13s -> %-13s |   %6.1f km      |\n",
                       camionesArreglo[i].id,
                       localidadesArreglo[origen].nombre,
                       localidadesArreglo[destino].nombre,
                       distancia_total);
            }
        }
    }
    else
    {
        printf("Modo invalido. Usa 1 para TIEMPO o 2 para DISTANCIA.\n");
    }

    printf("--------------------------------------------------------------\n");
}

void merge(int left, int centro, int right, ProductoSeleccionado a[])
{
    int tam = right - left + 1;
    ProductoSeleccionado *temp = malloc(tam * sizeof(ProductoSeleccionado));
    int i = left, j = centro + 1, k = 0;

    while (i <= centro && j <= right)
    {
        if (a[i].peso_total <= a[j].peso_total)
            temp[k++] = a[i++];
        else
            temp[k++] = a[j++];
    }

    while (i <= centro)
        temp[k++] = a[i++];

    while (j <= right)
        temp[k++] = a[j++];

    // Copiar el resultado ordenado de temp[] a a[]
    for (i = left, k = 0; i <= right; i++, k++)
        a[i] = temp[k];

    free(temp);
}

void mergeSort(int left, int right, ProductoSeleccionado a[])
{
    if (left < right)
    {
        int centro = (left + right) / 2;
        mergeSort(left, centro, a);
        mergeSort(centro + 1, right, a);
        merge(left, centro, right, a);
    }
}

void imprimir_arreglo(ProductoSeleccionado a[], int n)
{
    for (int i = 0; i < n; i++)
    {
        printf("Producto: %s | Peso: %.2f | Volumen: %.2f\n",
               a[i].producto->nombre,
               a[i].peso_total,
               a[i].volumen_total);
    }
}

int hash_nombre(const char *nombre)
{
    return (unsigned char)nombre[0] % HASH_SIZE;
}

int comparar_nombre(const void *a, const void *b)
{
    const ProductoSeleccionado *p1 = (const ProductoSeleccionado *)a;
    const ProductoSeleccionado *p2 = (const ProductoSeleccionado *)b;
    return strcmp(p1->producto->nombre, p2->producto->nombre);
}

void hash_sort_asignados_nombre(ProductoSeleccionado asignados[], int total)
{
    // Inicializar la tabla global
    for (int i = 0; i < HASH_SIZE; i++)
    {
        tablaHash[i] = NULL;
    }

    // Insertar en tabla hash
    for (int i = 0; i < total; i++)
    {
        int idx = hash_nombre(asignados[i].producto->nombre);
        NodoHash *nuevo = (NodoHash *)malloc(sizeof(NodoHash));
        nuevo->prod = asignados[i];
        nuevo->sig = tablaHash[idx];
        tablaHash[idx] = nuevo;
    }

    // Mostrar elementos por orden lexicográfico por bucket
    for (int i = 0; i < HASH_SIZE; i++)
    {
        NodoHash *actual = tablaHash[i];
        ProductoSeleccionado bucket[50];
        int count = 0;

        while (actual && count < 50)
        {
            bucket[count++] = actual->prod;
            actual = actual->sig;
        }

        if (count > 0)
        {
            qsort(bucket, count, sizeof(ProductoSeleccionado), comparar_nombre);
            for (int j = 0; j < count; j++)
            {
                printf("Producto: %s | Cantidad: %d | Peso: %.2f | Volumen: %.2f\n",
                       bucket[j].producto->nombre,
                       bucket[j].cantidad,
                       bucket[j].peso_total,
                       bucket[j].volumen_total);
            }
        }
    }
}

void liberar_tabla_hash()
{
    for (int i = 0; i < HASH_SIZE; i++)
    {
        NodoHash *actual = tablaHash[i];
        while (actual)
        {
            NodoHash *temp = actual;
            actual = actual->sig;
            free(temp);
        }
        tablaHash[i] = NULL;
    }
}

int particion_lomuto_peso(ProductoSeleccionado a[], int bajo, int alto)
{
    float pivote = a[alto].peso_total;
    int i = bajo - 1;

    for (int j = bajo; j < alto; j++)
    {
        if (a[j].peso_total <= pivote)
        {
            i++;
            ProductoSeleccionado temp = a[i];
            a[i] = a[j];
            a[j] = temp;
        }
    }

    ProductoSeleccionado temp = a[i + 1];
    a[i + 1] = a[alto];
    a[alto] = temp;

    return i + 1;
}

void quick_sort_lomuto_peso(ProductoSeleccionado a[], int bajo, int alto)
{
    if (bajo < alto)
    {
        int pi = particion_lomuto_peso(a, bajo, alto);
        quick_sort_lomuto_peso(a, bajo, pi - 1);
        quick_sort_lomuto_peso(a, pi + 1, alto);
    }
}

int obtener_max_volumen_entero(ProductoSeleccionado arr[], int n)
{
    int max = (int)(arr[0].volumen_total * 100);
    for (int i = 1; i < n; i++)
    {
        int val = (int)(arr[i].volumen_total * 100);
        if (val > max)
            max = val;
    }
    return max;
}

void contar_por_digito_volumen(ProductoSeleccionado arr[], int n, int exp)
{
    ProductoSeleccionado salida[MAX_PRODUCTOS];
    int cuenta[10] = {0};

    for (int i = 0; i < n; i++)
    {
        int val = (int)(arr[i].volumen_total * 100);
        cuenta[(val / exp) % 10]++;
    }

    for (int i = 1; i < 10; i++)
        cuenta[i] += cuenta[i - 1];

    for (int i = n - 1; i >= 0; i--)
    {
        int val = (int)(arr[i].volumen_total * 100);
        int idx = (val / exp) % 10;
        salida[--cuenta[idx]] = arr[i];
    }

    for (int i = 0; i < n; i++)
        arr[i] = salida[i];
}

void radix_sort_asignados_volumen(ProductoSeleccionado arr[], int n)
{
    if (n <= 1)
        return;

    int max_val = obtener_max_volumen_entero(arr, n);
    for (int exp = 1; max_val / exp > 0; exp *= 10)
        contar_por_digito_volumen(arr, n, exp);
}

void counting_sort_volumen(ProductoSeleccionado arr[], int n)
{
    if (n <= 1)
        return;

    int max_val = (int)(arr[0].volumen_total * 100);
    for (int i = 1; i < n; i++)
    {
        int val = (int)(arr[i].volumen_total * 100);
        if (val > max_val)
            max_val = val;
    }

    int count[max_val + 1];
    for (int i = 0; i <= max_val; i++)
        count[i] = 0;

    for (int i = 0; i < n; i++)
        count[(int)(arr[i].volumen_total * 100)]++;

    for (int i = 1; i <= max_val; i++)
        count[i] += count[i - 1];

    ProductoSeleccionado salida[n];
    for (int i = n - 1; i >= 0; i--)
    {
        salida[count[(int)(arr[i].volumen_total * 100)] - 1] = arr[i];
        count[(int)(arr[i].volumen_total * 100)]--;
    }

    for (int i = 0; i < n; i++)
        arr[i] = salida[i];
}

typedef struct
{
    ProductoSeleccionado *elementos; // Cambio: usar arreglo de ProductoSeleccionado
    int capacidad;
    int tamanio;
} Heap;

void init_heap(Heap *h, int capacidad)
{
    h->elementos = malloc(capacidad * sizeof(ProductoSeleccionado));
    h->capacidad = capacidad;
    h->tamanio = 0;
}

void swap_productos(ProductoSeleccionado *a, ProductoSeleccionado *b)
{
    ProductoSeleccionado temp = *a;
    *a = *b;
    *b = temp;
}

void heapify_nombres(Heap *h, int idx)
{
    int menor = idx;
    int izq = 2 * idx + 1;
    int der = 2 * idx + 2;

    // Comparar nombres usando strcmp
    if (izq < h->tamanio &&
        strcmp(h->elementos[izq].producto->nombre,
               h->elementos[menor].producto->nombre) < 0)
        menor = izq;

    if (der < h->tamanio &&
        strcmp(h->elementos[der].producto->nombre,
               h->elementos[menor].producto->nombre) < 0)
        menor = der;

    if (menor != idx)
    {
        swap_productos(&h->elementos[idx], &h->elementos[menor]);
        heapify_nombres(h, menor);
    }
}

void push_heap(Heap *h, ProductoSeleccionado producto)
{
    if (h->tamanio == h->capacidad)
    {
        printf("Heap lleno\n");
        return;
    }

    // Insertar al final
    int i = h->tamanio;
    h->elementos[i] = producto;
    h->tamanio++;

    // Flotar el elemento si es necesario
    while (i > 0 && strcmp(h->elementos[i].producto->nombre,
                           h->elementos[(i - 1) / 2].producto->nombre) < 0)
    {
        swap_productos(&h->elementos[i], &h->elementos[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

ProductoSeleccionado pop_heap(Heap *h)
{
    if (h->tamanio <= 0)
    {
        printf("Heap vacío\n");
        ProductoSeleccionado vacio = {0};
        return vacio;
    }

    ProductoSeleccionado raiz = h->elementos[0];
    h->elementos[0] = h->elementos[h->tamanio - 1];
    h->tamanio--;

    heapify_nombres(h, 0);
    return raiz;
}

void heap_sort_nombres(ProductoSeleccionado arr[], int n)
{
    // Crear y llenar el heap
    Heap h;
    init_heap(&h, n);

    for (int i = 0; i < n; i++)
    {
        push_heap(&h, arr[i]);
    }

    // Extraer elementos en orden
    for (int i = 0; i < n; i++)
    {
        arr[i] = pop_heap(&h);
    }

    free(h.elementos);
}

void submenu_ordenar_productos()
{
    int opcion;
    do
    {
        printf("\n\n===== UVS EXPRESS - MENU CATEGORIAS PRODUCTOS=====\n");
        printf("1. Ordenar por nombre (Aun no asignados)\n");
        printf("2. Ordenar por peso (Aun no asignados)\n");
        printf("3. Ordenar por volumen (Aun no asignados)\n");
        printf("4. Ordenar por nombre \n");
        printf("5. Ordenar por peso \n");
        printf("6. Ordenar por volumen \n");
        printf("0. Atras\n");
        printf("Selecciona una opcion: ");
        scanf("%d", &opcion);

        switch (opcion)
        {
        case 1:
            if (total_no <= 0)
            {
                printf("No hay productos no asignados para ordenar.\n");
                break;
            }
            printf("Ordenando por nombre (Aun no asignados)...\n");
            heap_sort_nombres(no_seleccionados, total_no);
            printf("\nProductos ordenados por nombre:\n");
            imprimir_arreglo(no_seleccionados, total_no);
            break;
        case 2:

            if (total_no <= 0)
            {
                printf("No hay productos no asignados para ordenar.\n");
                break;
            }
            printf("Ordenando por peso (Aun no asignados)...\n");
            mergeSort(0, total_no - 1, no_seleccionados);
            imprimir_arreglo(no_seleccionados, total_no);
            break;

        case 3:

            if (total_no <= 0)
            {
                printf("No hay productos no asignados para ordenar.\n");
                break;
            }
            printf("Ordenando por volumen (Aun no asignados)...\n");
            counting_sort_volumen(no_seleccionados, total_no);
            imprimir_arreglo(no_seleccionados, total_no);
            break;
        case 4:

            if (total_si <= 0)
            {

                printf("No hay productos asignados para ordenar.\n");
                break;
            }
            printf("Ordenando por nombre (asignados)...\n");
            hash_sort_asignados_nombre(si_seleccionados, total_si);
            liberar_tabla_hash();
            break;

        case 5:

            if (total_si <= 0)
            {

                printf("No hay productos asignados para ordenar.\n");
                break;
            }
            printf("Ordenando por peso (asignados)...\n");
            quick_sort_lomuto_peso(si_seleccionados, 0, total_si - 1);
            imprimir_arreglo(si_seleccionados, total_si);
            break;

        case 6:

            if (total_si <= 0)
            {

                printf("No hay productos asignados para ordenar.\n");
                break;
            }
            printf("Ordenando por volumen (asignados)...\n");
            radix_sort_asignados_volumen(si_seleccionados, total_si);
            imprimir_arreglo(si_seleccionados, total_si);
            break;

        case 7:

            if (total_si <= 0)
            {

                printf("No hay productos asignados .\n");
                break;
            }
            printf("mostrar seleccionados...\n");

            imprimir_arreglo(si_seleccionados, total_si);
            break;

        case 0:
            printf("Saliendo del submenu...\n");
            break;
        default:
            printf("Opcion invalida. Intenta de nuevo.\n");
        }
    } while (opcion != 0);
}

void menuPrincipal()
{
    int opcion;
    do
    {
        printf("\n\n===== UVS EXPRESS - MENU PRINCIPAL =====\n");
        printf("1. Asignar productos optimamente a camiones\n");
        printf("2. Optimizar rutas y asignar a camiones (modo tiempo)\n");
        printf("3. Optimizar rutas y asignar a camiones (modo distancia)\n");
        printf("4. Ordenar por...\n");
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
        case 4:
            submenu_ordenar_productos();
            break;
        case 0:
            printf("Saliendo del programa...\n");
            break;
        default:
            printf("Opcion invalida. Intenta de nuevo.\n");
        }
    } while (opcion != 0);
}

int main()
{
    precargarDatos();
    menuPrincipal();

    return 0;
}
