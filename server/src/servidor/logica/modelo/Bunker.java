package servidor.logica.modelo;

/**
 * Bunker (escudo de proteccion). Esta hecho de una grilla de FILAS x COLUMNAS
 * bloques pequeños. Cada bloque se destruye con un impacto.
 *
 * - Posicion (x,y) = esquina superior izquierda del bunker en pantalla.
 * - Cada bloque mide LADO_BLOQUE pixeles.
 * - El admin puede setear el porcentaje de bloques vivos con BUNKERS n%.
 *
 * Se serializa como:
 *   BUNKER id x y filas columnas lado bitmap
 * donde bitmap es FILAS*COLUMNAS caracteres '0' o '1' (1 = bloque vivo).
 */
public class Bunker {

    public static final int FILAS        = 3;
    public static final int COLUMNAS     = 5;
    public static final int LADO_BLOQUE  = 20;
    public static final int ANCHO        = COLUMNAS * LADO_BLOQUE; // 100
    public static final int ALTO         = FILAS    * LADO_BLOQUE; // 60
    public static final int TOTAL_BLOQUES = FILAS * COLUMNAS;       // 15

    private int id;
    private int x, y;
    private boolean[][] bloques;  // true = vivo, false = destruido

    public Bunker(int id, int x, int y) {
        this.id = id;
        this.x  = x;
        this.y  = y;
        this.bloques = new boolean[FILAS][COLUMNAS];
        setPorcentaje(100);
    }

    /**
     * Devuelve true y destruye el bloque si la bala (bx,by,bw,bh) impacta
     * algun bloque vivo. Devuelve false si no hay impacto.
     */
    public boolean impacto(int bx, int by, int bw, int bh) {
        for (int f = 0; f < FILAS; f++) {
            for (int c = 0; c < COLUMNAS; c++) {
                if (!bloques[f][c]) continue;
                int bx0 = x + c * LADO_BLOQUE;
                int by0 = y + f * LADO_BLOQUE;
                boolean choca = bx < bx0 + LADO_BLOQUE && bx + bw > bx0 &&
                                by < by0 + LADO_BLOQUE && by + bh > by0;
                if (choca) {
                    bloques[f][c] = false;
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * Cambia el porcentaje de bloques vivos del bunker.
     * Mantiene vivos los bloques mas centrales (los de afuera se "destruyen"
     * primero) para que el dano se vea natural — como si vinieran de fuera.
     */
    public void setPorcentaje(int pct) {
        if (pct < 0)   pct = 0;
        if (pct > 100) pct = 100;
        int vivos = Math.round(TOTAL_BLOQUES * pct / 100f);

        // orden[0] = bloque mas central; bloques externos al final.
        int[][] orden = ordenPorCentroPrimero();
        for (int i = 0; i < TOTAL_BLOQUES; i++) {
            int f = orden[i][0];
            int c = orden[i][1];
            // Los primeros 'vivos' (mas centrales) quedan; el resto destruido.
            bloques[f][c] = (i < vivos);
        }
    }

    // Lista de (f,c) ordenada de mas central a mas externo.
    private int[][] ordenPorCentroPrimero() {
        int[][] orden = new int[TOTAL_BLOQUES][2];
        Integer[][] tmp = new Integer[TOTAL_BLOQUES][3];
        int i = 0;
        double cf = (FILAS    - 1) / 2.0;
        double cc = (COLUMNAS - 1) / 2.0;
        for (int f = 0; f < FILAS; f++) {
            for (int c = 0; c < COLUMNAS; c++) {
                double d = Math.hypot(f - cf, c - cc);
                tmp[i][0] = (int)(d * 1000);  // distancia escalada a entero
                tmp[i][1] = f;
                tmp[i][2] = c;
                i++;
            }
        }
        // Orden ascendente: distancia chica (centro) primero.
        java.util.Arrays.sort(tmp, (a, b) -> a[0] - b[0]);
        for (int k = 0; k < TOTAL_BLOQUES; k++) {
            orden[k][0] = tmp[k][1];
            orden[k][1] = tmp[k][2];
        }
        return orden;
    }

    public int  getId()    { return id; }
    public int  getX()     { return x;  }
    public int  getY()     { return y;  }

    public String serializar() {
        StringBuilder sb = new StringBuilder();
        for (int f = 0; f < FILAS; f++)
            for (int c = 0; c < COLUMNAS; c++)
                sb.append(bloques[f][c] ? '1' : '0');
        return String.format("BUNKER %d %d %d %d %d %d %s",
            id, x, y, FILAS, COLUMNAS, LADO_BLOQUE, sb.toString());
    }
}
