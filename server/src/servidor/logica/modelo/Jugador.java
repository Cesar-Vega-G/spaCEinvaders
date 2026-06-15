package servidor.logica.modelo;

/**
 * Representa al jugador en el juego.
 * Paradigma OO: encapsula estado y comportamiento del jugador.
 */
public class Jugador {

    // Constantes
    public static final int ANCHO = 65;
    public static final int ALTO = 30;
    public static final int VELOCIDAD = 20;
    public static final int INICIO_X = 380;
    public static final int INICIO_Y = 810;
    public static final int VIDAS_INICIALES = 3;

    private int id;
    private int x;
    private int y;
    private int vidas;
    private int puntaje;

    public Jugador(int id) {
        this.id = id;
        this.x = INICIO_X + (id * 300);
        this.y = INICIO_Y;
        this.vidas = VIDAS_INICIALES;
        this.puntaje = 0;
    }

    public void moverIzquierda() {
        if (x > 0)
            x -= VELOCIDAD;
    }

    public void moverDerecha(int anchoPantalla) {
        if (x + ANCHO < anchoPantalla)
            x += VELOCIDAD;
    }

    public void perderVida() {
        vidas--;
    }

    public void ganarVida() {
        vidas++;
    }

    public void sumarPuntaje(int pts) {
        puntaje += pts;
    }

    public boolean estaVivo() {
        return vidas > 0;
    }

    // Getters
    public int getId() {
        return id;
    }

    public int getX() {
        return x;
    }

    public int getY() {
        return y;
    }

    public int getVidas() {
        return vidas;
    }

    public int getPuntaje() {
        return puntaje;
    }

    public String serializar() {
        return String.format("JUGADOR %d %d %d %d %d", id, x, y, vidas, puntaje);
    }
}
