import matplotlib.pyplot as plt
import numpy as np

tests = ["./graphs/5linearVelocity", "./graphs/10linearVelocity", "./graphs/20linearVelocity"]
colors = ["green","yellow", "red"]
labels = ['5 cm/s', '10 cm/s', '20 cm/s']

plt.style.use('petroff10')

for i in range(len(tests)):
    x = []
    y = []
    with open(tests[i]) as f:
        next(f)
        for line in f:
            values = line.strip().split(",")
            y.append(float(values[0]))
            x.append(float(values[1]))

    plt.scatter(
    x, y,
    color=colors[i],
    edgecolors="black",
    linewidths=0.9,
    label=labels[i],
    s=100
)

    plt.quiver(
        np.zeros(len(x)),
        np.zeros(len(y)),
        x, y,
        angles="xy",
        scale_units="xy",
        scale=1,
        width=0.002,
        color=colors[i]
    )

plt.xlim(-7.5, 7.5)
plt.ylim(35, 50)
plt.legend(title="Velocity", title_fontsize=12, fontsize=10)
plt.grid(True, zorder=3)

plt.xlabel("Deviation from target line [cm]")
plt.ylabel("Distance driving [cm]")
plt.title("Accuracy of going 50cm forward")
plt.show()