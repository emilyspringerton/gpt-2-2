import os
import sys
import pytest

# Add src to path so we can import the modules
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../src')))

import generate_unconditional_samples

def test_model_files_exist():
    '''Verifies that the model downloader populated the directory.'''
    model_name = '124M'
    base_dir = os.path.dirname(os.path.dirname(__file__)) # Go up one level from tests/
    model_dir = os.path.join(base_dir, 'models', model_name)
    
    required_files = [
        'checkpoint', 
        'encoder.json', 
        'hparams.json', 
        'model.ckpt.data-00000-of-00001', 
        'model.ckpt.index', 
        'model.ckpt.meta', 
        'vocab.bpe'
    ]
    
    for f in required_files:
        assert os.path.exists(os.path.join(model_dir, f)), f"Missing model file: {f}"

def test_smoke_generation():
    '''Runs a minimal generation to ensure TF graph builds without crashing.'''
    model_name = '124M'
    
    # We use very small parameters for speed
    try:
        generate_unconditional_samples.sample_model(
            model_name=model_name,
            length=5,       # Generate only 5 tokens
            nsamples=1,     # Generate only 1 sample
            batch_size=1,
            top_k=40,
            models_dir='models'
        )
    except Exception as e:
        pytest.fail(f"Model generation crashed: {e}")
