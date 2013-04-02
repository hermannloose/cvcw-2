# Maximally Stable Extremal Regions (MSER) #

## Usage ##

`mser -d delta [--mser_random_color] [--mser_rgb=color] [--mser_alpha=alpha] <input> <output> [logging.properties]`

## Options ##

<code>**-d** delta</code>

> Set the delta parameter for finding MSERs. You will want this to be roughly
> between 70 and 110.

<code>**--mser\_rgb**=0x112233</code>

> Set the RGB value used for coloring MSERs. Default: 0xFF0000 (red).

<code>**--mser\_alpha**=0xff</code>

> Set the alpha value used for coloring MSERs. Default: 0xFF (full opacity).

<code>**--mser\_random\_color**</code>

> Use a random RGB color for each MSER. `--mser_alpha` will still be used,
> `--mser_rgb` will be ignored.

## Example ##

`mser -d 100 zebra.jpg output.png logging.properties`
