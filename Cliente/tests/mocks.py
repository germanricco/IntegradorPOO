from unittest.mock import MagicMock, patch

class FakeServerProxy:
    """
    Imita el comportamiento de un ServerProxy de xmlrpc.
    Esto se hace asi porque no queremos que el servidor real interfiera en nuestros tests
    Permite interceptar llamadas explícitas a __getattr__.
    """
    def __init__(self, uri, allow_none=True):
        # Creamos diccionarios vacios para guardar las respuestas que queremos que el servidor devuelva
        self.respuestas = {}
        self.errores = {}

    def __getattr__(self, service_name):
        """
        Este método es llamado por ApiClient cuando hace:
        self.api.__getattr__("auth.login")
        """
        # Creamos un Mock que actuará como la función remota (ej. auth.login)
        mock_service = MagicMock()

        # 1. Revisamos si tenemos un error configurado para este método
        # Ejemplo: si "auth.login" está en "errores" se ejecuta este bloque
        if service_name in self.errores:
            # side_effect es para lanzar errores
            mock_service.side_effect = self.errores[service_name]
            return mock_service

        # 2. Revisamos si tenemos una respuesta exitosa configurada
        if service_name in self.respuestas:
            # return_value es para devolver lo que nosotros queramos
            mock_service.return_value = self.respuestas[service_name]
            return mock_service

        # 3. Retorno por defecto (vacío) si no configuramos nada
        return mock_service
