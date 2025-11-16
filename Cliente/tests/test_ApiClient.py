import pytest
from unittest.mock import patch
from xmlrpc.client import Fault
import sys
import os

# Ajuste de path para importar el módulo Cliente
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..')))

from Cliente.ApiClient import APIClient
from Cliente.tests.mocks import FakeServerProxy

# ==== FIXTURE ====
@pytest.fixture
def client_mock():
    """
    Inicializa APIClient usando nuestra FakeServerProxy en lugar de la real.
    """
    # Interceptamos la CREACIÓN del ServerProxy real
    # Con esto le decimos que no use la clase real, sino que use a FakeServerProxy
    with patch('Cliente.ApiClient.ServerProxy', side_effect=FakeServerProxy) as MockClass:
        
        # Inicializamos el cliente (esto creará internamente un FakeServerProxy)
        client = APIClient("http://localhost:8080")
        
        # Obtenemos acceso a la instancia falsa que se creó dentro del cliente
        fake_server_instance = client.api
        
        yield client, fake_server_instance

# ==== TESTS ====
def test_login_success(client_mock):
    """Prueba el flujo del login."""
    client, fake_server = client_mock
    
    # 1. Configurar la respuesta esperada en nuestro servidor falso
    respuesta_esperada = {
        "token": "token_falso_123", 
        "user": "admin_test", 
        "privilegio": "admin"
    }
    
    # Le decimos al servidor falso: "Cuando pidan 'auth.login', devuelve este diccionario"
    fake_server.respuestas["auth.login"] = respuesta_esperada

    # 2. Ejecutar
    result = client.login("admin", "1234")

    # 3. Validar (assert te dice básicamente si algo es true o false)
    assert result["success"] is True
    assert client.token == "token_falso_123"
    assert client.user == "admin_test"

def test_login_failure(client_mock):
    """Prueba el flujo de error en el login."""
    client, fake_server = client_mock

    # 1. Configurar el error esperado
    # Le decimos al servidor falso: "Cuando pidan 'auth.login', lanza este error"
    fake_server.errores["auth.login"] = Fault(1, "Credenciales invalidas")

    # 2. Ejecutar
    result = client.login("usuario_erroneo", "clave_mal")

    # 3. Validar
    assert result["success"] is False
    assert "Credenciales invalidas" in result["error"]
    assert client.token is None