
class HelloWorld {
	public HelloWorld(String[] args) {
		System.out.println("Hello, World!\n");
		for (int i=0; i < args.length; i++) 
			System.out.println("argument #" + i + " = " + args[i]);
		System.out.println();
		System.out.println();
	}
}