namespace intro {
  enqueue pc

  // Remove Japanese text from title screen.
  seek($AFD794)
  fill $38

  dequeue pc
}
