package servidor.logica;

/**
 * PATRÓN OBSERVER — Interfaz que deben implementar
 * todos los que quieran recibir actualizaciones del estado del juego.
 */
public interface ObservadorEstado {
    void actualizar(String estadoSerializado);
}