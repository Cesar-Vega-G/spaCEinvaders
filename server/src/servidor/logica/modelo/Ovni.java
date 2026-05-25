package servidor.logica.modelo;

import java.util.Random;

/**
 * Representa el OVNI que aparece cada cierto tiempo.
 * Puntos aleatorios según el enunciado.
 */
public class Ovni {

    public static final int ANCHO  = 60;
    public static final int ALTO   = 30;
    public static final int Y_FIJO = 30;

    private static final Random random = new Random();

    private int x, y;
    private int velocidad;
    private int direccion; // 1 = derecha, -1 = izquierda
    private int puntos;
    private boolean activo;

    public Ovni() {
        this.y      = Y_FIJO;
        this.activo = false;
    }

    public void aparecer(int anchoPantalla, int direccion, int puntos) {
        this.direccion = direccion;
        this.puntos    = puntos > 0 ? puntos : (random.nextInt(6) + 1) * 50;
        this.velocidad = 6;
        this.activo    = true;
        this.x         = direccion == 1 ? -ANCHO : anchoPantalla;
    }

    public void actualizar(int anchoPantalla) {
        if (!activo) return;
        x += velocidad * direccion;
        if (x + ANCHO < 0 || x > anchoPantalla) activo = false;
    }

    public void destruir() { activo = false; }

    public boolean colisionaCon(int bx, int by, int bw, int bh) {
        return activo &&
               bx < x + ANCHO && bx + bw > x &&
               by < y + ALTO  && by + bh  > y;
    }

    // Getters
    public int getX()         { return x; }
    public int getY()         { return y; }
    public boolean isActivo() { return activo; }
    public int getPuntos()    { return puntos; }

    public String serializar() {
        return String.format("OVNI %d %d %d %d", x, y, activo ? 1 : 0, puntos);
    }
}
