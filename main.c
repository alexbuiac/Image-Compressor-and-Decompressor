#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// structura pentru rgb
typedef struct colors {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} colors;

// structura pentru nodurile arborilor
typedef struct node {
    unsigned char is_leaf;
    colors pixel;
    struct node *tl;    // top-left
    struct node *tr;    // top-right
    struct node *br;    // bottom-right
    struct node *bl;    // nottom-left
} tree_node;

// structura pentru nod din coada folosita la decompresie
typedef struct q_node {
    tree_node **data;
    struct q_node *next;
} q_node;

// coada
typedef struct queue {
    q_node *front, *rear;
} queue;

// functie pentru adaugarea unui nod nod in coada
q_node *new_node(tree_node **ptr)
{
    // se aloca memorie, se atribuie adresa unui element din arbore
    // si se intoarce adresa nodului pentru coada
    q_node *temp = (q_node *)malloc(sizeof(q_node));
    temp->data = ptr;
    temp->next = NULL;
    return temp;
}

// functie pentru initerea cozii
queue *create_queue()
{
    queue *q = (queue *)malloc(sizeof(queue));
    q->front = q->rear = NULL;
    return q;
}

// adaugarea unui element in coada
queue *enqueue(queue *q, tree_node **ptr)
{
    q_node *temp = new_node(ptr);
    
    // daca coada e nula, front si rear vor retine aceeasi adresa
    if (q->rear == NULL)
    {
        q->front = q->rear = temp;
        return q;
    }
    
    // daca nu, se adauga elementul la finalul cozii
    q->rear->next = temp;
    q->rear = temp;
    return q;
}

// functie pentru scoaterea unui element din coada
queue *dequeue(queue *q)
{
    // se scoate primul element din coada si se elibereaza memoria
    if (q->front == NULL)
    {
        return q;
    }
    q_node *temp = q->front;
    q->front = q->front->next;
    if (q->front == NULL)
    {
        q->rear = NULL;
    }
    free(temp);
    return q;
}

// functie care verifica daca coada este goala
int empty_q(queue *q)
{
    if (q->front == NULL)
    {
        return 1;
    }
    else
    {    
        return 0;
    }
}

// functie pentru eliberarea memoriei folosite de coada
queue *free_queue(queue *q)
{
    if (!empty_q(q))
    {
        return q;
    }
    while (q->front != NULL)
    {
        q_node *temp = q->front;
        q->front = q->front->next;
        free(temp);
    }
    return q;
}

// functie pentru crearea unui nou nod in arbore
tree_node *create_node(colors pixel, unsigned char is_leaf)
{
    // se aloca memorie si se atribuie proprietatile nodului
    tree_node *node = (tree_node *)malloc(sizeof(tree_node));
    node->is_leaf = is_leaf;
    node->pixel = pixel;
    node->tl = NULL;
    node->tr = NULL;
    node->br = NULL;
    node->bl = NULL;
    return node;
}

// functii pentru calcularea culorilor medii intr-o zona a imaginii
// (incepand de la punctul (x, y) pana la (x + size, y + size))
unsigned char calculate_red(colors **image, unsigned int size, int x, int y)
{
    unsigned long long sum_red = 0;
    int i, j;
    for (i = x; i < x + size; i++)
    {
        for (j = y; j < y + size; j++)
        {
            sum_red += image[i][j].red;
        }
    }
    unsigned char red = sum_red / (size * size);
    return red;
}

unsigned char calculate_green(colors **image, unsigned int size, int x, int y)
{
    int i, j;
    unsigned long long sum_green = 0;
    for (i = x; i < x + size; i++)
    {
        for (j = y; j < y + size; j++)
        {
            sum_green += image[i][j].green;
        }
    }
    unsigned char green = sum_green / (size * size);
    return green;
}

unsigned char calculate_blue(colors **image, unsigned int size, int x, int y)
{
    int i, j;
    unsigned long long sum_blue = 0;
    for (i = x; i < x + size; i++)
    {
        for (j = y; j < y + size; j++)
        {
            sum_blue += image[i][j].blue;
        }
    }
    unsigned char blue = sum_blue / (size * size);
    return blue;
}

// functie pentru calcularea scorului de similaritate
unsigned long long calculate_mean(colors **image, unsigned int size, int x, 
                                  int y, unsigned long long red,
                                  unsigned long long green,
                                  unsigned long long blue)
{
    unsigned long long sum_mean = 0;
    int i, j;
    for (i = x; i < x + size; i++)
    {
        for (j = y; j < y + size; j++)
        {
            sum_mean += ((red - image[i][j].red) * (red - image[i][j].red)) +
                        ((green - image[i][j].green) * (green - image[i][j].green)) +
                        ((blue - image[i][j].blue) * (blue - image[i][j].blue));
        }
    }

    unsigned long long mean = sum_mean / (3 * (size * size));
    return mean;
}

// funcite pentru construirea recursiva a arborelui
tree_node *generate_tree(colors **image, unsigned int size, int x, int y,
                         int factor, int *count, int *max_size,
                         int *nr_of_nodes)
{
    // se calculeaza culorile medii pe zona de imagine 
    // reprezentata prin x, y, size
    unsigned char red = calculate_red(image, size, x, y);
    unsigned char green = calculate_green(image, size, x, y);
    unsigned char blue = calculate_blue(image, size, x, y);
    
    // se calculeaza scorul de similaritate
    unsigned long long mean = calculate_mean(image, size, x, y, red, green,
                                             blue);
    
    // daca scorul este mai mic sau egal cu eroarea, bucata de imagine
    // devine frunza in arborele nostru, iesindu-se din recursivitate
    if (mean <= factor || size == 1)
    {
        if (size > *max_size)
        {
            *max_size = size;
        }
        (*count)++;
        (*nr_of_nodes)++;
        colors pixel;
        pixel.red = red;
        pixel.green = green;
        pixel.blue = blue;
        return create_node(pixel, 1);
    } 
    // daca nu, calculam in mod recursiv scorurile de similaritate
    // pentru toti copiii nodului curent
    else
    {
        colors pixel;
        pixel.red = 0;
        pixel.green = 0;
        pixel.blue = 0;
        tree_node *node = create_node(pixel, 0);
        (*nr_of_nodes)++;
        int child_size = size / 2;
        node->tl = generate_tree(image, child_size, x, y, factor, count,
                                 max_size, nr_of_nodes);
        node->tr = generate_tree(image, child_size, x, y + child_size, factor,
                                 count, max_size, nr_of_nodes);
        node->br = generate_tree(image, child_size, x + child_size,
                                 y + child_size, factor, count, max_size,
                                 nr_of_nodes);
        node->bl = generate_tree(image, child_size, x + child_size, y, factor,
                                 count, max_size, nr_of_nodes);
        return node;
    }
    return NULL;
}

// functie pentru calcularea recursiva a adancimii arborelui
int calculate_tree_depth(tree_node *root)
{
    // daca nodul e null, adancimea e 0
    if (root == NULL)
    {
        return 0;
    }
    else
    {
        // se cauta recursiv maximul dintre adancimile celor 4 copii
        int max_depth = 0;
        int depth;
        depth = calculate_tree_depth(root->tl);
        if (depth > max_depth)
        {
            max_depth = depth;
        }

        depth = calculate_tree_depth(root->tr);
        if (depth > max_depth)
        {
            max_depth = depth;
        }

        depth = calculate_tree_depth(root->br);
        if (depth > max_depth)
        {
            max_depth = depth;
        }

        depth = calculate_tree_depth(root->bl);
        if (depth > max_depth)
        {
            max_depth = depth;
        }
        
        // adancimea arborelui e maximul dintre adancimile copiilor + 1
        return max_depth + 1;
    }
}

// functie pentru eliberarea recursiva a memoriei folosite de arbore
void free_tree(tree_node *root)
{
    if (root == NULL)
    {
        return;
    }
    free_tree(root->tl);
    free_tree(root->tr);
    free_tree(root->br);
    free_tree(root->bl);
    free(root);
}

// functie pentru generarea fisierului comprimat
void write_compressed_file(tree_node *root, FILE *out, int nr_of_nodes, int size)
{
    fwrite(&size, sizeof(unsigned int), 1, out);
    
    // se foloseste o coada pentru a retine adresele nodurilor care
    // nu sunt frunze
    tree_node **queue = (tree_node **)malloc(nr_of_nodes * sizeof(tree_node *));
    int front = 0;
    int back = 0;
    
    // se adauga radacina in coada
    queue[back] = root;
    back++;

    // cat timp coada nu este goala, se parcurge arborele
    while (front < back)
    {
        // se extrage primul element din coada
        tree_node *curr_node = queue[front];
        front++;

        // daca nodul curent este frunza
        // se scriu datele in fisier
        if (curr_node->is_leaf)
        {
            unsigned char type = curr_node->is_leaf;
            unsigned char red = curr_node->pixel.red;
            unsigned char green = curr_node->pixel.green;
            unsigned char blue = curr_node->pixel.blue;
            fwrite(&type, sizeof(unsigned char), 1, out);
            fwrite(&red, sizeof(unsigned char), 1, out);
            fwrite(&green, sizeof(unsigned char), 1, out);
            fwrite(&blue, sizeof(unsigned char), 1, out);
        }
        else
        {
            unsigned char type = curr_node->is_leaf;
            fwrite(&type, sizeof(unsigned char), 1, out);
        }
        
        // se adauga in coada toti copiii nodului curent
        if (curr_node->tl != NULL)
        {
            queue[back] = curr_node->tl;
            back++;
        }
        if (curr_node->tr != NULL)
        {
            queue[back] = curr_node->tr;
            back++;
        }
        if (curr_node->br != NULL)
        {
            queue[back] = curr_node->br;
            back++;
        }
        if (curr_node->bl != NULL)
        {
            queue[back] = curr_node->bl;
            back++;
        }
    }
    // se elibereaza memoria folosita pentru coada
    free(queue);
}

// functie pentru construirea alborelui folossit la decompresie
void build_decompressed_tree(FILE *in, tree_node **root)
{
    // se muta cursorul la inceputul fisierului
    fseek(in, sizeof(unsigned int), SEEK_SET);
    unsigned char is_leaf;
    colors pixel;
    
    // se initializeaza coada si se pune in ea adresa radacinii
    queue *q = create_queue();
    q = enqueue(q, root);
    tree_node **curr;
    
    // cat timp coada nu este goala, parcurgem fisierul comprimat
    while (!empty_q(q))
    {
        // extragem primul element din coada
        curr = q->front->data;
        
        // citim tipul nodului
        fread(&is_leaf, sizeof(unsigned char), 1, in);
        
        // daca nodul este frunza, se citesc valorile culorile si
        // se creeaza un nod frunza in arbore
        if (is_leaf)
        {
            fread(&pixel, sizeof(colors), 1, in);
            *curr = create_node(pixel, 1);
        }
        // daca nu, se creeaza un nod intermediar in arbore
        else
        {
            pixel.red = pixel.green = pixel.blue = 0;
            *curr = create_node(pixel, 0);
            q = enqueue(q, &((*curr)->tl));
            q = enqueue(q, &((*curr)->tr));
            q = enqueue(q, &((*curr)->br));
            q = enqueue(q, &((*curr)->bl));
        }
        q = dequeue(q);
    }
    
    // se elibereaza memoria folosita pentru coada
    free_queue(q);
    free(q);
    return;
}

// functie pentru construirea recursiva a imaginii, plecand de la arbore
void make_image(colors **image, tree_node *root, unsigned int size, int x, int y)
{
    // daca nodul curent e de tip frunza, zona corespunzatoare
    // din imagine va avea culorile retinute in arbore
    if (root->is_leaf)
    {
        int i, j;
        for (i = x; i < x + size; i++)
        {
            for (j = y; j < y + size; j++)
            {
                image[i][j] = root->pixel;
            }
        }
    }
    // daca nu, se apeleaza recursiv functia pe toti copiii nodului
    // pana se ajunge la frunze
    else
    {
        int child_size = size / 2;
        make_image(image, root->tl, child_size, x, y);
        make_image(image, root->tr, child_size, x, y + child_size);
        make_image(image, root->br, child_size, x + child_size, y + child_size);
        make_image(image, root->bl, child_size, x + child_size, y);
    }
}

// functie pentru construirea fisierului .ppm
void write_decompressed_file(FILE *out, colors **image, unsigned int size)
{
    // se scriu in format text tipul fisierului, dimensiunea imaginii
    // si valoarea maxima pe care o pot lua culorile
    fprintf(out, "P6\n");
    fprintf(out, "%d %d\n", size, size);
    fprintf(out, "255\n");
    
    // se scriu in format binar valorile culorilor din matrice
    int i;
    for (i = 0; i < size; i++)
    {
        fwrite(image[i], sizeof(colors), size, out);
    }
}

int main(int argc, char *argv[])
{
    // cerintele 1 si 2
    if (!strcmp(argv[1], "-c1") || !strcmp(argv[1], "-c2"))
    {
        // se converteste string-ul dat ca argument in valoarea
        // int pentru factor
        char *p;
        int factor = strtol(argv[2], &p, 10);
        
        // se deschide fisierul .ppm si se citesc datele despre imagine
        FILE *in = fopen(argv[3], "rb");
        char type[2];
        unsigned int size, size_2;
        int max_color_value;
        fscanf(in, "%s", type);
        fscanf(in, "%d %d", &size, &size_2);
        fscanf(in, "%d", &max_color_value);
        fgetc(in);

        // se aloca memorie pentru matricea in care retinem culorile
        // si se citesc din fisier
        colors **image = (colors **)malloc(size * sizeof(colors *));
        int i, j;
        for (i = 0; i < size; i++)
        {
            image[i] = (colors *)malloc(size * sizeof(colors));
        }
        for (i = 0; i < size; i++)
        {
            for (j = 0; j < size; j++)
            {
                fread(&image[i][j], sizeof(colors), 1, in);
            }
        }
        
        // generam arborele de compresie, numaram numarul de frunze
        // si cautam dimensiunea maxima a unui nod frunza
        int count = 0;
        int max_size = 0;
        int nr_of_nodes = 0;
        tree_node *root = generate_tree(image, size, 0, 0, factor, &count,
                                        &max_size, &nr_of_nodes);
        
        // cerinta 1
        if (!strcmp(argv[1], "-c1"))
        {
            // se deschide fisierul si se scriu datele necesare
            FILE *out = fopen(argv[4], "w");
            fprintf(out, "%d\n%d\n%d\n", calculate_tree_depth(root), count,
                                          max_size);
            fclose(out);
        }
        // cerinta 2
        else
        {
            // se deschide fisierul si se scriu datele
            FILE *out = fopen(argv[4], "wb");
            write_compressed_file(root, out, nr_of_nodes, size);
            fclose(out);
        }
        
        // se elibereaza toata memoria ramasa in folosinta
        for (i = 0; i < size; i++)
        {
             free(image[i]);
        }
        free(image);
        free_tree(root);
        fclose(in);
    }
    // cerinta 3
    else if (!strcmp(argv[1], "-d"))
    {
        // se deschide fisierul comprimat si se citeste dimensiunea imaginii
        FILE *in = fopen(argv[2], "rb");
        unsigned int size;
        fread(&size, sizeof(unsigned int), 1, in);
        
        // se genereaza arborele
        tree_node *root = NULL;
        build_decompressed_tree(in, &root);
        
        // se deschide fisierul de iesire
        // se realizeaza imaginea si se scriu datele in fisier
        FILE *out = fopen(argv[3], "wb");
        colors **image = (colors **)malloc(size * sizeof(colors *));
        int i;
        for (i = 0; i < size; i++)
        {
            image[i] = (colors *)malloc(size * sizeof(colors));
        }
        make_image(image, root, size, 0, 0);
        write_decompressed_file(out, image, size);
        
        // se elibereaza memoria ramasa in folosinta si se inchid fisierele
        free_tree(root);
        for (i = 0; i < size; i++)
        {
            free(image[i]);
        }
        free(image);
        fclose(in);
        fclose(out);
    }
    return 0;
}