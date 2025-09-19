#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Cliente PyQt5 para Sistema Distribuído - Contador de Letras/Números
Interface gráfica para comunicação com o servidor mestre
"""

import sys
import json
import os
from typing import Dict, Any, Optional
from pathlib import Path

import requests
from PyQt5.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
    QGroupBox, QLabel, QLineEdit, QSpinBox, QPushButton, QTextEdit,
    QFileDialog, QMessageBox, QStatusBar, QDialog, QDialogButtonBox,
    QSplitter, QFrame
)
from PyQt5.QtCore import Qt, QThread, pyqtSignal, QTimer
from PyQt5.QtGui import QFont, QIcon




class ProcessingWorker(QThread):
    """Worker thread para processamento assíncrono"""

    finished = pyqtSignal(dict)
    error = pyqtSignal(str)

    def __init__(self, url: str, data: Dict[str, Any]):
        super().__init__()
        self.url = url
        self.data = data

    def run(self):
        try:
            response = requests.post(
                self.url,
                json=self.data,
                headers={'Content-Type': 'application/json'},
                timeout=30
            )

            if response.status_code == 200:
                result = response.json()
                self.finished.emit(result)
            else:
                self.error.emit(f"Erro HTTP {response.status_code}: {response.text}")

        except requests.exceptions.RequestException as e:
            self.error.emit(f"Erro de conexão: {str(e)}")
        except Exception as e:
            self.error.emit(f"Erro inesperado: {str(e)}")


class ServerConfigWidget(QGroupBox):
    """Widget para configuração do servidor"""

    def __init__(self):
        super().__init__("Configuração do Servidor")
        self.setup_ui()

    def setup_ui(self):
        layout = QHBoxLayout()

        # Host
        layout.addWidget(QLabel("Servidor:"))
        self.host_edit = QLineEdit("localhost")
        self.host_edit.setMaximumWidth(150)
        layout.addWidget(self.host_edit)

        # Porta
        layout.addWidget(QLabel("Porta:"))
        self.port_spin = QSpinBox()
        self.port_spin.setRange(1, 65535)
        self.port_spin.setValue(8080)
        self.port_spin.setMaximumWidth(80)
        layout.addWidget(self.port_spin)

        # Botões
        self.config_button = QPushButton("Configurar")
        self.status_button = QPushButton("Verificar Status")
        layout.addWidget(self.config_button)
        layout.addWidget(self.status_button)

        layout.addStretch()
        self.setLayout(layout)

    def get_server_url(self) -> str:
        return f"http://{self.host_edit.text()}:{self.port_spin.value()}"


class ProcessingWidget(QGroupBox):
    """Widget para opções de processamento"""

    def __init__(self):
        super().__init__("Processamento")
        self.setup_ui()

    def setup_ui(self):
        layout = QHBoxLayout()

        self.file_button = QPushButton("Processar Arquivo")
        self.text_button = QPushButton("Processar Texto")

        layout.addWidget(self.file_button)
        layout.addWidget(self.text_button)
        layout.addStretch()

        self.setLayout(layout)


class TextInputWidget(QGroupBox):
    """Widget para entrada de texto"""

    def __init__(self):
        super().__init__("Entrada de Texto")
        self.setup_ui()

    def setup_ui(self):
        layout = QVBoxLayout()

        layout.addWidget(QLabel("Digite o texto a ser processado:"))

        self.text_edit = QTextEdit()
        self.text_edit.setMaximumHeight(150)
        self.text_edit.setPlaceholderText("Digite seu texto aqui...")
        layout.addWidget(self.text_edit)

        self.process_button = QPushButton("Processar Este Texto")
        layout.addWidget(self.process_button)

        self.setLayout(layout)

    def get_text(self) -> str:
        return self.text_edit.toPlainText()

    def clear_text(self):
        self.text_edit.clear()


class ResultsWidget(QGroupBox):
    """Widget para exibição de resultados"""

    def __init__(self):
        super().__init__("Resultados")
        self.setup_ui()

    def setup_ui(self):
        layout = QVBoxLayout()

        self.results_text = QTextEdit()
        self.results_text.setReadOnly(True)
        self.results_text.setFont(QFont("Courier", 10))
        layout.addWidget(self.results_text)

        self.setLayout(layout)

    def show_results(self, text: str):
        self.results_text.setPlainText(text)
        # Rolar para o topo
        cursor = self.results_text.textCursor()
        cursor.movePosition(cursor.Start)
        self.results_text.setTextCursor(cursor)


class TextInputDialog(QDialog):
    """Dialog para entrada de texto"""

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Processar Texto")
        self.setModal(True)
        self.resize(400, 300)
        self.setup_ui()

    def setup_ui(self):
        layout = QVBoxLayout()

        layout.addWidget(QLabel("Digite o texto a ser processado:"))

        self.text_edit = QTextEdit()
        self.text_edit.setPlaceholderText("Digite seu texto aqui...")
        layout.addWidget(self.text_edit)

        # Botões
        buttons = QDialogButtonBox(
            QDialogButtonBox.Ok | QDialogButtonBox.Cancel,
            Qt.Horizontal
        )
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        layout.addWidget(buttons)

        self.setLayout(layout)

    def get_text(self) -> str:
        return self.text_edit.toPlainText()


class ClientWindow(QMainWindow):
    """Janela principal do cliente"""

    def __init__(self):
        super().__init__()
        self.worker = None
        self.setup_ui()
        self.connect_signals()
        self.show_initial_message()

    def setup_ui(self):
        self.setWindowTitle("Cliente Sistema Distribuído - Contador de Letras/Números")
        self.setGeometry(100, 100, 900, 700)

        # Widget central
        central_widget = QWidget()
        self.setCentralWidget(central_widget)

        # Layout principal
        main_layout = QVBoxLayout()
        central_widget.setLayout(main_layout)

        # Layout superior (configuração e processamento)
        top_layout = QHBoxLayout()

        # Configuração do servidor
        self.server_config = ServerConfigWidget()
        top_layout.addWidget(self.server_config)

        # Processamento
        self.processing = ProcessingWidget()
        top_layout.addWidget(self.processing)

        main_layout.addLayout(top_layout)

        # Splitter para área de texto e resultados
        splitter = QSplitter(Qt.Vertical)

        # Entrada de texto
        self.text_input = TextInputWidget()
        splitter.addWidget(self.text_input)

        # Resultados
        self.results = ResultsWidget()
        splitter.addWidget(self.results)

        # Configurar proporções do splitter
        splitter.setSizes([200, 400])
        main_layout.addWidget(splitter)

        # Barra de status
        self.status_bar = QStatusBar()
        self.setStatusBar(self.status_bar)
        self.update_status("Pronto. Servidor: http://localhost:8080")


    def connect_signals(self):
        """Conectar sinais dos widgets"""
        self.server_config.config_button.clicked.connect(self.configure_server)
        self.server_config.status_button.clicked.connect(self.check_server_status)
        self.processing.file_button.clicked.connect(self.process_file)
        self.processing.text_button.clicked.connect(self.process_text_dialog)
        self.text_input.process_button.clicked.connect(self.process_text_area)

    def show_initial_message(self):
        """Mostrar mensagem inicial"""
        initial_text = """Cliente iniciado.
Servidor configurado: http://localhost:8080

Pronto para processar arquivos e textos."""
        self.results.show_results(initial_text)

    def configure_server(self):
        """Configurar endereço do servidor"""
        url = self.server_config.get_server_url()
        self.update_status(f"Servidor configurado: {url}")
        self.results.show_results(f"Servidor configurado para: {url}")

    def check_server_status(self):
        """Verificar status do servidor"""
        url = self.server_config.get_server_url()
        self.update_status("Verificando status do servidor...")

        try:
            response = requests.get(f"{url}/health", timeout=5)

            if response.status_code == 200:
                self.update_status(f"Servidor operacional: {url}")
                self.results.show_results(
                    f"✅ Servidor está operacional e pronto para processar requisições!\n"
                    f"URL: {url}"
                )
            else:
                self.update_status(f"Servidor com problemas: {url}")
                self.results.show_results(
                    f"❌ Servidor respondeu com status {response.status_code}.\n"
                    f"URL: {url}"
                )

        except requests.exceptions.RequestException as e:
            self.update_status(f"Servidor não disponível: {url}")
            self.results.show_results(
                f"❌ Servidor não está disponível ou com problemas.\n"
                f"Erro: {str(e)}\n"
                f"Verifique se o servidor está rodando em: {url}"
            )

    def process_file(self):
        """Processar arquivo de texto"""
        file_path, _ = QFileDialog.getOpenFileName(
            self,
            "Selecionar arquivo de texto",
            "",
            "Arquivos de texto (*.txt);;Todos os arquivos (*)"
        )

        if file_path:
            try:
                with open(file_path, 'r', encoding='utf-8') as f:
                    content = f.read()

                self.process_text_content(content, f"Arquivo: {Path(file_path).name}")

            except Exception as e:
                self.show_error(f"Erro ao ler arquivo: {str(e)}")

    def process_text_dialog(self):
        """Processar texto via dialog"""
        dialog = TextInputDialog(self)

        if dialog.exec_() == QDialog.Accepted:
            text = dialog.get_text()
            if text.strip():
                self.process_text_content(text, f"Texto ({len(text)} caracteres)")
            else:
                self.show_error("Texto não pode estar vazio!")

    def process_text_area(self):
        """Processar texto da área de entrada"""
        text = self.text_input.get_text()
        if text.strip():
            self.process_text_content(text, f"Texto ({len(text)} caracteres)")
        else:
            self.show_error("Texto não pode estar vazio!")

    def process_text_content(self, text: str, source: str):
        """Processar conteúdo de texto"""
        url = f"{self.server_config.get_server_url()}/process"
        data = {"text": text}

        self.update_status(f"Processando {source.lower()}...")

        # Desabilitar botões durante processamento
        self.set_buttons_enabled(False)

        # Criar e iniciar worker thread
        self.worker = ProcessingWorker(url, data)
        self.worker.finished.connect(lambda result: self.on_processing_finished(result, source))
        self.worker.error.connect(self.on_processing_error)
        self.worker.start()

    def on_processing_finished(self, result: Dict[str, Any], source: str):
        """Callback para processamento finalizado"""
        self.set_buttons_enabled(True)
        self.display_result(result, source)

    def on_processing_error(self, error_message: str):
        """Callback para erro no processamento"""
        self.set_buttons_enabled(True)
        self.update_status(f"Erro: {error_message}")
        self.results.show_results(f"❌ Erro no processamento:\n{error_message}")

    def display_result(self, result: Dict[str, Any], source: str):
        """Exibir resultado do processamento"""
        output = []
        output.append("=" * 60)
        output.append("RESULTADO DO PROCESSAMENTO")
        output.append(f"Fonte: {source}")
        output.append("=" * 60)
        output.append("")

        if result.get('success', False):
            output.append("✅ Processamento concluído com sucesso!")
            output.append("")

            letters = result.get('letters_count', 0)
            numbers = result.get('numbers_count', 0)
            total_chars = result.get('total_characters', 0)
            time_ms = result.get('processing_time_ms', 0)

            output.append("📊 ESTATÍSTICAS:")
            output.append(f"   Letras encontradas:     {letters:8}")
            output.append(f"   Números encontrados:    {numbers:8}")
            output.append(f"   Total de caracteres:    {total_chars:8}")
            output.append(f"   Tempo de processamento: {time_ms:8.2f} ms")

            # Calcular percentuais
            total = letters + numbers
            if total > 0:
                letter_pct = (letters / total) * 100.0
                number_pct = (numbers / total) * 100.0

                output.append("")
                output.append("📈 DISTRIBUIÇÃO:")
                output.append(f"   Letras:  {letter_pct:6.2f}%")
                output.append(f"   Números: {number_pct:6.2f}%")

            self.update_status("Processamento concluído com sucesso!")

        else:
            output.append("❌ Falha no processamento!")
            output.append("")
            error_msg = result.get('error_message', 'Erro desconhecido')
            output.append(f"💥 ERRO: {error_msg}")

            raw_response = result.get('raw_response', '')
            if raw_response:
                output.append("")
                output.append("📄 Resposta do servidor:")
                output.append(raw_response[:500])
                if len(raw_response) > 500:
                    output.append("... (truncado)")

            self.update_status(f"Erro no processamento: {error_msg}")

        output.append("")
        output.append("=" * 60)

        self.results.show_results("\n".join(output))

    def set_buttons_enabled(self, enabled: bool):
        """Habilitar/desabilitar botões"""
        self.server_config.config_button.setEnabled(enabled)
        self.server_config.status_button.setEnabled(enabled)
        self.processing.file_button.setEnabled(enabled)
        self.processing.text_button.setEnabled(enabled)
        self.text_input.process_button.setEnabled(enabled)

    def update_status(self, message: str):
        """Atualizar barra de status"""
        self.status_bar.showMessage(message)

    def show_error(self, message: str):
        """Mostrar mensagem de erro"""
        QMessageBox.critical(self, "Erro", message)
        self.update_status(f"Erro: {message}")

    def closeEvent(self, event):
        """Evento de fechamento da janela"""
        if self.worker and self.worker.isRunning():
            self.worker.terminate()
            self.worker.wait()
        event.accept()


def main():
    """Função principal"""
    app = QApplication(sys.argv)
    app.setApplicationName("Cliente Sistema Distribuído")
    app.setApplicationVersion("2.0")

    # Configurar estilo
    app.setStyle("Fusion")

    # Criar e mostrar janela principal
    window = ClientWindow()
    window.show()

    # Executar aplicação
    sys.exit(app.exec_())


if __name__ == "__main__":
    main()