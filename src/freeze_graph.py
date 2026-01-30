import os
import json
import tensorflow as tf
from tensorflow.python.framework import graph_util
import model, sample, encoder

def freeze_model(model_name='124M', models_dir='models', export_dir='export'):
    '''
    Loads the GPT-2 model and exports a frozen graph (.pb) for C/C++ usage.
    '''
    models_dir = os.path.expanduser(os.path.expandvars(models_dir))
    batch_size = 1
    length = 32 # Fixed buffer size for C binding simplicity

    enc = encoder.get_encoder(model_name, models_dir)
    hparams = model.default_hparams()
    with open(os.path.join(models_dir, model_name, 'hparams.json')) as f:
        hparams.override_from_dict(json.load(f))

    with tf.Session(graph=tf.Graph()) as sess:
        # Define Input Tensor (Placeholder)
        context = tf.placeholder(tf.int32, [batch_size, None], name='input_context')

        # Build Graph
        output = sample.sample_sequence(
            hparams=hparams, 
            length=length,
            context=context,
            batch_size=batch_size,
            temperature=1.0, top_k=0, top_p=1.0
        )
        
        # Name the output node for C API access
        output = tf.identity(output, name='output_logits')

        # Restore Weights
        saver = tf.train.Saver()
        ckpt = tf.train.latest_checkpoint(os.path.join(models_dir, model_name))
        saver.restore(sess, ckpt)

        # Freeze Graph (Convert Variables to Constants)
        output_graph_def = graph_util.convert_variables_to_constants(
            sess,
            sess.graph_def,
            ['output_logits'] # Output node names
        )

        # Save
        if not os.path.exists(export_dir):
            os.makedirs(export_dir)
        
        out_path = os.path.join(export_dir, f"gpt2_{model_name}_frozen.pb")
        with tf.gfile.GFile(out_path, "wb") as f:
            f.write(output_graph_def.SerializeToString())
        
        print(f"✅ Frozen Graph exported to: {out_path}")
        print(f"   - Input Node: 'input_context' (int32)")
        print(f"   - Output Node: 'output_logits' (int32)")

if __name__ == '__main__':
    freeze_model()
