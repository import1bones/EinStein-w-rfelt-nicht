"""
Modern Neural Network Module for Einstein Game
Uses TensorFlow 2.x with modern architecture and training techniques
"""

import numpy as np
import tensorflow as tf
from tensorflow import keras
from tensorflow.keras import layers, Sequential, Model
import json
import os
from typing import Optional, Tuple, List
import logging

class EinsteinNeuralNetwork:
    """
    Modern neural network for Einstein game position evaluation.
    
    Features:
    - Residual connections for better gradient flow
    - Batch normalization for training stability
    - Dropout for regularization
    - Configurable architecture
    """
    
    def __init__(self, config_path: str = "config.json"):
        """Initialize the neural network with configuration."""
        self.logger = logging.getLogger(__name__)
        self.config = self._load_config(config_path)
        self.model: Optional[Model] = None
        self.is_trained = False
        
    def _load_config(self, config_path: str) -> dict:
        """Load configuration from JSON file."""
        try:
            with open(config_path, 'r') as f:
                config = json.load(f)
            return config.get('ai', {}).get('neural_network', {})
        except FileNotFoundError:
            self.logger.warning(f"Config file {config_path} not found, using defaults")
            return self._get_default_config()
    
    def _get_default_config(self) -> dict:
        """Get default configuration."""
        return {
            "input_size": 150,  # 5x5 board x 6 history states
            "hidden_layers": [600, 600, 300],
            "output_size": 6,
            "dropout_rate": 0.2,
            "use_batch_norm": True,
            "use_residual": True
        }
    
    def build_model(self) -> Model:
        """Build the neural network model."""
        input_size = self.config.get("input_size", 150)
        hidden_layers = self.config.get("hidden_layers", [600, 600, 300])
        output_size = self.config.get("output_size", 6)
        dropout_rate = self.config.get("dropout_rate", 0.2)
        use_batch_norm = self.config.get("use_batch_norm", True)
        use_residual = self.config.get("use_residual", True)
        
        # Input layer
        inputs = keras.Input(shape=(input_size,), name="board_state")
        x = inputs
        
        # Hidden layers with optional residual connections
        for i, units in enumerate(hidden_layers):
            # Store input for potential residual connection
            residual = x
            
            # Dense layer
            x = layers.Dense(units, name=f"dense_{i}")(x)
            
            # Batch normalization
            if use_batch_norm:
                x = layers.BatchNormalization(name=f"bn_{i}")(x)
            
            # Activation
            x = layers.ReLU(name=f"relu_{i}")(x)
            
            # Dropout
            x = layers.Dropout(dropout_rate, name=f"dropout_{i}")(x)
            
            # Residual connection (if dimensions match)
            if use_residual and residual.shape[-1] == x.shape[-1]:
                x = layers.Add(name=f"residual_{i}")([x, residual])
        
        # Output layer
        outputs = layers.Dense(output_size, activation='softmax', name="output")(x)
        
        # Create model
        model = Model(inputs=inputs, outputs=outputs, name="EinsteinNet")
        
        # Compile model
        model.compile(
            optimizer=keras.optimizers.Adam(learning_rate=0.001),
            loss='categorical_crossentropy',
            metrics=['accuracy', 'top_k_categorical_accuracy']
        )
        
        self.model = model
        return model
    
    def load_weights(self, weights_path: str) -> bool:
        """Load pre-trained weights."""
        try:
            if self.model is None:
                self.build_model()
            
            if os.path.exists(weights_path):
                self.model.load_weights(weights_path)
                self.is_trained = True
                self.logger.info(f"Weights loaded from {weights_path}")
                return True
            else:
                self.logger.warning(f"Weights file not found: {weights_path}")
                return False
        except Exception as e:
            self.logger.error(f"Error loading weights: {e}")
            return False
    
    def save_weights(self, weights_path: str) -> bool:
        """Save model weights."""
        try:
            if self.model is not None:
                # Create directory if it doesn't exist
                os.makedirs(os.path.dirname(weights_path), exist_ok=True)
                self.model.save_weights(weights_path)
                self.logger.info(f"Weights saved to {weights_path}")
                return True
            else:
                self.logger.error("No model to save")
                return False
        except Exception as e:
            self.logger.error(f"Error saving weights: {e}")
            return False
    
    def predict(self, board_states: np.ndarray) -> np.ndarray:
        """
        Predict the best move for given board states.
        
        Args:
            board_states: Array of shape (batch_size, 150) containing board states
            
        Returns:
            Array of predicted move probabilities
        """
        if self.model is None or not self.is_trained:
            # Return random uniform distribution if no trained model
            batch_size = board_states.shape[0]
            return np.random.uniform(0, 1, (batch_size, self.config.get("output_size", 6)))
        
        try:
            # Ensure input is properly shaped
            if len(board_states.shape) == 1:
                board_states = board_states.reshape(1, -1)
            
            # Predict
            predictions = self.model.predict(board_states, verbose=0)
            return predictions
            
        except Exception as e:
            self.logger.error(f"Error during prediction: {e}")
            # Return uniform distribution as fallback
            batch_size = board_states.shape[0]
            return np.random.uniform(0, 1, (batch_size, self.config.get("output_size", 6)))
    
    def get_best_move(self, board_state: np.ndarray) -> int:
        """Get the best move index for a single board state."""
        prediction = self.predict(board_state.reshape(1, -1))
        return int(np.argmax(prediction[0]))
    
    def train(self, X: np.ndarray, y: np.ndarray, 
              validation_split: float = 0.2,
              epochs: int = 100,
              batch_size: int = 32) -> dict:
        """
        Train the neural network.
        
        Args:
            X: Training features of shape (samples, 150)
            y: Training labels of shape (samples, 6)
            validation_split: Fraction of data to use for validation
            epochs: Number of training epochs
            batch_size: Training batch size
            
        Returns:
            Training history dictionary
        """
        if self.model is None:
            self.build_model()
        
        # Callbacks
        callbacks = [
            keras.callbacks.EarlyStopping(
                monitor='val_loss',
                patience=10,
                restore_best_weights=True
            ),
            keras.callbacks.ReduceLROnPlateau(
                monitor='val_loss',
                factor=0.5,
                patience=5,
                min_lr=1e-7
            )
        ]
        
        # Train model
        history = self.model.fit(
            X, y,
            validation_split=validation_split,
            epochs=epochs,
            batch_size=batch_size,
            callbacks=callbacks,
            verbose=1
        )
        
        self.is_trained = True
        return history.history
    
    def evaluate(self, X_test: np.ndarray, y_test: np.ndarray) -> dict:
        """Evaluate model performance."""
        if self.model is None or not self.is_trained:
            return {"error": "Model not trained"}
        
        results = self.model.evaluate(X_test, y_test, verbose=0)
        metric_names = self.model.metrics_names
        
        return dict(zip(metric_names, results))


# Global instance for C++ integration
_global_network: Optional[EinsteinNeuralNetwork] = None

def initialize_network(config_path: str = "config.json") -> bool:
    """Initialize the global neural network instance."""
    global _global_network
    try:
        _global_network = EinsteinNeuralNetwork(config_path)
        _global_network.build_model()
        return True
    except Exception as e:
        logging.error(f"Failed to initialize network: {e}")
        return False

def load_weights(weights_path: str) -> bool:
    """Load weights into the global network."""
    global _global_network
    if _global_network is None:
        return False
    return _global_network.load_weights(weights_path)

def get_move_prediction(board_state: List[float]) -> int:
    """
    Get move prediction for C++ integration.
    
    Args:
        board_state: List of 150 floats representing the board state
        
    Returns:
        Best move index (0-5)
    """
    global _global_network
    if _global_network is None:
        return 0  # Fallback
    
    try:
        board_array = np.array(board_state, dtype=np.float32)
        return _global_network.get_best_move(board_array)
    except Exception as e:
        logging.error(f"Error in move prediction: {e}")
        return 0  # Fallback

def cleanup_network():
    """Cleanup the global network."""
    global _global_network
    _global_network = None

if __name__ == "__main__":
    # Example usage
    logging.basicConfig(level=logging.INFO)
    
    # Create network
    network = EinsteinNeuralNetwork()
    network.build_model()
    
    print("Model summary:")
    network.model.summary()
    
    # Test with random data
    test_input = np.random.random((1, 150))
    prediction = network.predict(test_input)
    best_move = network.get_best_move(test_input.flatten())
    
    print(f"Test prediction: {prediction}")
    print(f"Best move: {best_move}")
