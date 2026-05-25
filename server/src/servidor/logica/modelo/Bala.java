package servidor.logica.modelo;

/**
 * Representa la bala disparada por un jugador.
 */
public class Bala {

    public static final int ANCHO     = 5;
    public static final int ALTO      = 15;
    public static final int VELOCIDAD = 15;

    private int x, y;
    private boolean activa;
    private int idJugador;

    public Bala(int idJugador) {
        this.idJugador = idJugador;
        this.activa    = false;
    }

    public void disparar(int x, int y) {
        if (!activa) {
            this.x = x;
            this.y = y;
            this.activa = true;
        }
    }

    public void actualizar() {
        if (activa) {
            y -= VELOCIDAD;
            if (y + ALTO < 0) activa = false;
        }
    }

    public void desactivar() { activa = false; }

    // Getters
    public int getX()         { return x; }
    public int getY()         { return y; }
    public boolean isActiva() { return activa; }
    public int getIdJugador() { return idJugador; }

    public String serializar() {
        return String.format("BALA %d %d %d %d", idJugador, x, y, activa ? 1 : 0);
    }
}
