import singularity_trainer

t = singularity_trainer.make_trainer(open("programs/asd.json", 'r').read())
t.step_batch()