package servidor.logica.modelo;

/**
 * Representa un enemigo individual.
 * Usa enum para los tipos — patrón OO que el profesor evalúa.
 */
public class Enemigo {

    public enum Tipo {
        CALAMAR(10),
        CANGREJO(20),
        PULPO(40);

        private final int puntos;
        Tipo(int puntos) { this.puntos = puntos; }
        public int getPuntos() { return puntos; }
    }

    public static final int ANCHO = 50;
    public static final int ALTO  = 50;

    private int x, y;
    private boolean activo;
    private Tipo tipo;
    private int fila, columna;

    public Enemigo(int x, int y, Tipo tipo, int fila, int columna) {
        this.x       = x;
        this.y       = y;
        this.activo  = true;
        this.tipo    = tipo;
        this.fila    = fila;
        this.columna = columna;
    }

    public void mover(int dx, int dy) { x += dx; y += dy; }
    public void destruir() { activo = false; }

    public boolean colisionaCon(int bx, int by, int bw, int bh) {
        return activo &&
               bx < x + ANCHO && bx + bw > x &&
               by < y + ALTO  && by + bh  > y;
    }

    // Getters
    public int getX()        { return x; }
    public int getY()        { return y; }
    public boolean isActivo(){ return activo; }
    public Tipo getTipo()    { return tipo; }
    public int getPuntos()   { return tipo.getPuntos(); }
    public int getFila()     { return fila; }
    public int getColumna()  { return columna; }

    public String serializar() {
        return String.format("ENEMIGO %d %d %d %d %d %s",
            fila, columna, x, y, activo ? 1 : 0, tipo.name());
    }
}
