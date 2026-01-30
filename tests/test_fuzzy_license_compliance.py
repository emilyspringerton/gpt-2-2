
import os
import sys
import pytest
import random
import logging
from unittest.mock import patch
import tensorflow as tf

# Add src to path
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../src')))

import generate_unconditional_samples
import model

# --- LICENSE CONFIGURATION ---
LICENSE_ATTRIBUTION = "⚠️  COMPLIANCE LOG: Content created using GPT-2."
LICENSE_HEADER = \"\"\"
Modified MIT License
Software Copyright (c) 2019 OpenAI
We only ask that you use GPT-2 responsibly and clearly indicate your content was created using GPT-2.
\"\"\"

# Setup Logger
logger = logging.getLogger("GPT2_Fuzzy_Test")
logger.setLevel(logging.INFO)
handler = logging.StreamHandler(sys.stdout)
handler.setFormatter(logging.Formatter('%(message)s'))
logger.addHandler(handler)

class Fuzzer:
    @staticmethod
    def get_weird_params():
        '''Generates weird but valid parameters to stress-test the loader'''
        return {
            'temperature': random.choice([0.01, 0.7, 1.0, 1.5, 3.0]), # Extremes
            'top_k': random.choice([0, 1, 40, 100]), # Deterministic vs Chaos
            'length': random.randint(1, 15), # Short bursts
            'seed': random.randint(0, 999999)
        }

def setup_module(module):
    '''Print License Header once at start of tests'''
    print("\n" + "="*60)
    print(LICENSE_HEADER)
    print("="*60 + "\n")

@pytest.mark.parametrize("execution_number", range(3)) # Run 3 fuzzy iterations
def test_fuzzy_model_loading_and_attribution(execution_number, capsys):
    '''
    Fuzzy test that verifies model loading under unstable parameters
    and enforces license attribution logging.
    '''
    # 1. Generate Fuzzy Parameters
    params = Fuzzer.get_weird_params()
    model_name = '124M'
    
    logger.info(f"\n🧪 [Fuzzy Run #{execution_number}] Params: {params}")

    # 2. Check Model Exists
    models_dir = 'models'
    if not os.path.exists(os.path.join(models_dir, model_name)):
        pytest.skip("Model 124M not found. Skipping.")

    # 3. Run Generation (Wrapped)
    # We patch stdout/print to capture the raw model output for attribution
    try:
        # Clear previous tensorflow graphs to prevent memory leaks during loops
        tf.reset_default_graph()
        
        generate_unconditional_samples.sample_model(
            model_name=model_name,
            nsamples=1,
            batch_size=1,
            models_dir=models_dir,
            **params
        )
        
        # Capture the output from the script
        captured = capsys.readouterr()
        raw_output = captured.out
        
        # 4. Enforce Attribution (The "Proper Logs")
        if raw_output:
            # We strip the separator lines often printed by the script
            clean_text = raw_output.replace("=" * 40, "").strip()
            
            logger.info(f"\n{LICENSE_ATTRIBUTION}")
            logger.info(f"📝 GENERATED ARTIFACT:\n{'-'*20}\n{clean_text}\n{'-'*20}")
            
            # Assertions to prove loading worked
            assert len(clean_text) > 0, "Model loaded but produced no text (Entropy Failure?)"
        else:
            pytest.fail("Model produced no output stream.")

    except Exception as e:
        logger.error(f"💥 FUZZ FAIL on params {params}: {e}")
        raise e

