package servidor.logica.modelo;

/**
 * Modelo de un enemigo individual en la cuadrícula o como extra del admin.
 *
 * Paradigma OO — enum anidado:
 *   El enum Tipo encapsula tanto el nombre como los puntos de cada categoría,
 *   eliminando switch/if dispersos y facilitando agregar nuevos tipos.
 *
 * Colisión:
 *   Se usa AABB (Axis-Aligned Bounding Box): dos rectángulos se solapan si
 *   sus rangos en X e Y se intersectan simultaneamente.
 */
public class Enemigo {

    /** Tipos de enemigo con sus puntos correspondientes (clásico Space Invaders). */
    public enum Tipo {
        CALAMAR (10),   /* fila superior: más difícil de golpear, menos puntos  */
        CANGREJO(20),   /* filas medias */
        PULPO   (40);   /* filas inferiores: más fáciles, más puntos */

        private final int puntos;
        Tipo(int puntos) { this.puntos = puntos; }
        public int getPuntos() { return puntos; }
    }

    public static final int ANCHO = 50; /* píxeles lógicos */
    public static final int ALTO  = 50;

    private int  x, y;
    private boolean activo;
    private Tipo tipo;
    private int  fila, columna; /* posición en la grilla para serialización */

    public Enemigo(int x, int y, Tipo tipo, int fila, int columna) {
        this.x       = x;
        this.y       = y;
        this.activo  = true;
        this.tipo    = tipo;
        this.fila    = fila;
        this.columna = columna;
    }

    /* El bloque entero se mueve; cada enemigo ajusta su posición aquí. */
    public void mover(int dx, int dy) { x += dx; y += dy; }

    public void destruir() { activo = false; }

    /* Colisión AABB: rect del proyectil (bx,by,bw,bh) contra este enemigo. */
    public boolean colisionaCon(int bx, int by, int bw, int bh) {
        return activo &&
               bx < x + ANCHO && bx + bw > x &&
               by < y + ALTO  && by + bh  > y;
    }

    public int     getX()       { return x;      }
    public int     getY()       { return y;      }
    public boolean isActivo()   { return activo; }
    public Tipo    getTipo()    { return tipo;   }
    public int     getPuntos()  { return tipo.getPuntos(); }
    public int     getFila()    { return fila;   }
    public int     getColumna() { return columna;}

    /* Formato: "ENEMIGO fila columna x y 0|1 TIPO" */
    public String serializar() {
        return String.format("ENEMIGO %d %d %d %d %d %s",
            fila, columna, x, y, activo ? 1 : 0, tipo.name());
    }
}
